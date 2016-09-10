/*
* Copyright (c) 1998 by Tech Soft 3D, LLC.
* The information contained herein is confidential and proprietary to
* Tech Soft 3D, LLC., and considered a trade secret as defined under
* civil and criminal statutes.	 Tech Soft 3D shall pursue its civil
* and criminal remedies in the event of unauthorized use or misappropriation
* of its trade secrets.  Use of this information by anyone other than
* authorized employees of Tech Soft 3D, LLC. is granted only under a
* written non-disclosure agreement, expressly prescribing the scope and
* manner of such use.
*
* $Id: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/utility/huffman.cpp#1 $
*/


#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vlist.h"
#include "vhash.h"

#ifdef HOOPS_DEFINED
#define VList HVList
#define VHash HVHash
#endif

#ifndef UNREFERENCED
#define UNREFERENCED(param) ((void)(param))
#endif

typedef VHash<unsigned short, int> VHashDictionary;


struct huffman_encode_table_t{
	unsigned short value;
	unsigned short count;
};

typedef VHash<unsigned short, huffman_encode_table_t*> EncodeTable;

struct huffman_decode_table_8_t{
	unsigned char value;
	unsigned char count;
};

struct huffman_decode_table_16_t{
	unsigned short value;
	unsigned char count;
};

struct huffman_tree_node_t {
	int weight;
	unsigned short value;
	struct huffman_tree_node_t *left;
	struct huffman_tree_node_t *right;
};

typedef VList<huffman_tree_node_t*> VListHuffTree;


struct huffman_encoder_8_t{
	VHashDictionary *histogram;

	huffman_decode_table_8_t *decode_table;
	EncodeTable *encode_table;
					
	int decode_table_size;
	int decode_bits;
 
};

struct huffman_encoder_16_t{
	VHashDictionary *histogram;
	huffman_decode_table_16_t *decode_table;

	EncodeTable *encode_table;
				
	int decode_table_size;
	int decode_bits;
 
};

static int sort_hufftree_by_weight(huffman_tree_node_t *a, huffman_tree_node_t *b, void const * const data)
{
	UNREFERENCED(data);

	if (a->weight == b->weight)
		return 0;
	else if (a->weight > b->weight)
		return 1;
	else
		return -1;
}

huffman_encoder_8_t * new_huffman_encoder_8() 
{
	huffman_encoder_8_t *he = (huffman_encoder_8_t *)malloc(sizeof(huffman_encoder_8_t));
	he->histogram = new VHashDictionary();
	he->encode_table = new EncodeTable();
	he->decode_table = 0;
	return he;
}

huffman_encoder_16_t * new_huffman_encoder_16() 
{
	huffman_encoder_16_t *he = (huffman_encoder_16_t *)malloc(sizeof(huffman_encoder_16_t));
	he->histogram = new VHashDictionary();
	he->encode_table = new EncodeTable();
	he->decode_table = 0;
	return he;
}


void delete_huffman_encoder_8(huffman_encoder_8_t *he) 
{
	delete he->histogram;
	delete [] he->decode_table;
	he->encode_table->DeleteItems();
	delete he->encode_table;

	free(he);
}

void delete_huffman_encoder_16(huffman_encoder_16_t *he) 
{
	delete he->histogram;
	delete [] he->decode_table;
	he->encode_table->DeleteItems();
	delete he->encode_table;

	free(he);
}

void huffman_add_data_8(huffman_encoder_8_t *he, int in_n, const char *in_data)
{
	while(in_n--) {
		unsigned short temp = (unsigned short)*in_data;
		int count = he->histogram->Lookup(temp);
		if (count == 0)
			he->histogram->InsertItem(temp, 1);
		else
			he->histogram->ReplaceItem(temp,++count);
		in_data++;
	}
}

void huffman_add_data_16(huffman_encoder_16_t *he, int in_n, const unsigned short *in_data)
{
	while(in_n--) {
		unsigned short temp = *in_data;
		int count = he->histogram->Lookup(temp);
		if (count == 0)
			he->histogram->InsertItem(temp, 1);
		else
			he->histogram->ReplaceItem(temp,++count);
		in_data++;
	}
}

static void chop_hufftree(huffman_tree_node_t *root) {
	if (root->left)
		chop_hufftree(root->left);
	if (root->right)
		chop_hufftree(root->right);
	delete root;
}

void huffman_create_tables_8(huffman_encoder_8_t *he)

{
	he->decode_table_size=2048;
	he->decode_bits=0;

	VListHuffTree *hufftree = new VListHuffTree();

	VHashDictionary::Pair *item;
	VHashDictionary::PairList *dictionary = he->histogram->GetPairList();
	dictionary->ResetCursor();
	int dictionary_length = dictionary->Count();
	for (int i=0; i<dictionary_length; i++) {
		item = dictionary->PeekCursor();
		huffman_tree_node_t *newnode = new huffman_tree_node_t;
		newnode->value = item->Key();
		newnode->weight = item->Item();
		newnode->left = newnode->right = 0;
		hufftree->AddSorted(newnode, sort_hufftree_by_weight);

		dictionary->AdvanceCursor();
	}
	delete dictionary;

	while (hufftree->Count() > 1) {
		huffman_tree_node_t *newnode = new huffman_tree_node_t;
		newnode->left = hufftree->RemoveFirst();
		newnode->right = hufftree->RemoveFirst();
		newnode->weight = newnode->left->weight + newnode->right->weight;
		newnode->value = 0; //internal nodes
		hufftree->AddSorted(newnode, sort_hufftree_by_weight);
	}

	huffman_tree_node_t *root = hufftree->PeekFirst();
	int temp=he->decode_table_size;
	for (int i=0; i<temp; i++) {
		/* find depth and make encode table */
		huffman_tree_node_t *walker = root;
		int this_decode_bits = 0;
		int bitblob = i;

		while (walker->left || walker->right) {
			if (bitblob & 1)
				walker = walker->right;
			else
				walker = walker->left;
			bitblob >>= 1;
			this_decode_bits++;
		}

		if (this_decode_bits > he->decode_bits)
			he->decode_bits = this_decode_bits;

		if (!he->encode_table->LookupItem(walker->value)) {
			huffman_encode_table_t *encode_table_item = new huffman_encode_table_t;
			encode_table_item->value = (unsigned short)(i & ((1 << this_decode_bits)-1));
			encode_table_item->count = (unsigned short)this_decode_bits;
			he->encode_table->InsertItem(walker->value, encode_table_item);
		}
	}

	he->decode_table_size = 1;
	he->decode_table_size <<= he->decode_bits; //must accommodate 2^(decode_bits) codes
	he->decode_table = new huffman_decode_table_8_t[he->decode_table_size];

	temp=he->decode_table_size;
	for (int i=0; i<temp; i++) {
		/* now make decode table */
		huffman_tree_node_t *walker = root;
		int this_decode_bits = 0;
		int bitblob = i;

		while (walker->left || walker->right) {
			if (bitblob & 1)
				walker = walker->right;
			else
				walker = walker->left;
			bitblob >>= 1;
			this_decode_bits++;
		}

		he->decode_table[i].value = (unsigned char)walker->value;
		he->decode_table[i].count = (unsigned char)this_decode_bits;
	}
	//clean up tree structure
	chop_hufftree(hufftree->PeekFirst());
	delete hufftree;

	delete he->histogram;
	he->histogram=0;
}

void huffman_create_tables_16(huffman_encoder_16_t *he)
{
	he->decode_table_size=65536;
	he->decode_bits=0;

	VListHuffTree *hufftree = new VListHuffTree();

	VHashDictionary::Pair *item;
	VHashDictionary::PairList *dictionary = he->histogram->GetPairList();
	dictionary->ResetCursor();
	int dictionary_length = dictionary->Count();
	for (int i=0; i<dictionary_length; i++) {
		item = dictionary->PeekCursor();
		huffman_tree_node_t *newnode = new huffman_tree_node_t;
		newnode->value = item->Key();
		newnode->weight = item->Item();
		newnode->left = newnode->right = 0;
		hufftree->AddSorted(newnode, sort_hufftree_by_weight);

		dictionary->AdvanceCursor();
	}
	delete dictionary;

	while (hufftree->Count() > 1) {
		huffman_tree_node_t *newnode = new huffman_tree_node_t;
		newnode->left = hufftree->RemoveFirst();
		newnode->right = hufftree->RemoveFirst();
		newnode->weight = newnode->left->weight + newnode->right->weight;
		newnode->value = 0; //internal nodes
		hufftree->AddSorted(newnode, sort_hufftree_by_weight);
	}

	huffman_tree_node_t *root = hufftree->PeekFirst();
	int temp=he->decode_table_size;
	for (int i=0; i<temp; i++) {
		/* find depth and make encode table */
		huffman_tree_node_t *walker = root;
		int this_decode_bits = 0;
		int bitblob = i;

		while (walker->left || walker->right) {
			if (bitblob & 1)
				walker = walker->right;
			else
				walker = walker->left;
			bitblob >>= 1;
			this_decode_bits++;
		}

		if (this_decode_bits > he->decode_bits)
			he->decode_bits = this_decode_bits;

		if (!he->encode_table->LookupItem(walker->value)) {
			huffman_encode_table_t * encode_table_item = new huffman_encode_table_t;
			encode_table_item->value = (unsigned short)(i & ((1 << this_decode_bits)-1));
			encode_table_item->count = (unsigned short)this_decode_bits;
			he->encode_table->InsertItem(walker->value, encode_table_item);
		}
	}

	he->decode_table_size = 1;
	he->decode_table_size <<= he->decode_bits; //must accommodate 2^(decode_bits) codes
	temp=he->decode_table_size;
	he->decode_table = new huffman_decode_table_16_t[temp];
	for (int i=0; i<temp; i++) {
		/* now make decode table */
		huffman_tree_node_t *walker = root;
		int this_decode_bits = 0;
		int bitblob = i;

		while (walker->left || walker->right) {
			if (bitblob & 1)
				walker = walker->right;
			else
				walker = walker->left;
			bitblob >>= 1;
			this_decode_bits++;
		}

		he->decode_table[i].value = (unsigned short)walker->value;
		he->decode_table[i].count = (unsigned char)this_decode_bits;
	}
	//clean up tree structure
	chop_hufftree(hufftree->PeekFirst());
	delete hufftree;
	
	delete he->histogram;
	he->histogram=0;
}

bool huffman_encode_8(huffman_encoder_8_t *he,
					int in_n, const char *in_data,
					int *out_data_bits, char *out_data)
{
	if (he->decode_bits > 16)
		return false;

	*out_data_bits=0;
	memset(out_data, 0, sizeof(char) * (in_n));
	
	int i=0;
	while (1) {
		huffman_encode_table_t *code = he->encode_table->Lookup((unsigned char)in_data[i]);
		if (!code)
			return false; //code not found in dictionary; cannot encode
		unsigned int value = (unsigned int)code->value;
		int count = (int)code->count;
		int index = *out_data_bits/8;
		int offset = *out_data_bits%8;
		*out_data_bits += count;

		if ((*out_data_bits + 7) / 8 > in_n)
			return false;

		if (in_n - index > 3) {
			unsigned int * block = (unsigned int *)&out_data[index];
			*block |= value << offset;
		}
		else if (in_n - index == 3) {
			unsigned short * block1 = (unsigned short *)&out_data[index];
			unsigned char * block2 = (unsigned char *) (&out_data[index] + 2);
			unsigned int shifted = value << offset;
			*block1 |= shifted;
			*block2 |= shifted >> 16;
		}
		else if (in_n - index == 2) {
			unsigned short * block = (unsigned short *)&out_data[index];
			*block |= value << offset;
		}
		else if (in_n - index == 1) {
			unsigned char * block = (unsigned char *)&out_data[index];
			*block |= value << offset;
		}

		if (++i == in_n)
			break;
	}

	return true;
}

bool huffman_encode_16(huffman_encoder_16_t *he,
					int in_n,	const unsigned short *in_data,
					int *out_data_bits, unsigned short *out_data)
{
	if (he->decode_bits > 16)
		return false;

	*out_data_bits=0;
	memset(out_data, 0, sizeof(unsigned short) * (in_n));

	int i=0;
	while (1) {
		huffman_encode_table_t *code = he->encode_table->Lookup((unsigned short)in_data[i]);
		if (!code)
			return false; //code not found in dictionary; cannot encode
		unsigned int value = (unsigned int)code->value;
		int count = (int)code->count;
		int index = *out_data_bits/16;
		int offset = *out_data_bits%16;
		*out_data_bits += count;

		if ((*out_data_bits + 15) / 16 > in_n)
			return false;

		if (in_n - index > 3) {
			unsigned int * block = (unsigned int *)&out_data[index];
			*block |= value << offset;
		}
		else if (in_n - index == 3) {
			unsigned short * block1 = (unsigned short *)&out_data[index];
			unsigned char * block2 = (unsigned char *) (&out_data[index] + 1);
			unsigned int shifted = value << offset;
			*block1 |= shifted;
			*block2 |= shifted >> 16;
		}
		else if (in_n - index == 2) {
			unsigned short * block = (unsigned short *)&out_data[index];
			*block |= value << offset;
		}
		else if (in_n - index == 1) {
			unsigned char * block = (unsigned char *)&out_data[index];
			*block |= value << offset;
		}

		if (++i == in_n)
			break;
	}

	return true;
}

void huffman_decode_8(huffman_encoder_8_t *he, int in_data_bits, const char *in_data, int *out_n, char *out_data) 
{
	int decoder_mask = (1<<he->decode_bits)-1;
	int current_bits = 0;
	int current = 0;
	int counted_bits = 0;
	int n=0;
	int in_n = (in_data_bits + 7) / 8;

	*out_n=0;

	while (1) {

		huffman_decode_table_8_t *entry; 

		while (current_bits < he->decode_bits) {

			if (n++ >= in_n) 
				break;

			current |= (((unsigned char)*in_data++) << current_bits);
			current_bits += 8;
		}

		entry = &he->decode_table[current & decoder_mask];

		current >>= entry->count;

		current_bits -= entry->count;

		out_data[(*out_n)++] = entry->value;

		if ( (counted_bits += entry->count) == in_data_bits)  //have we counted all the bits?
			break;

	}
}

void huffman_decode_16(huffman_encoder_16_t *he, int in_data_bits, const unsigned short *in_data, int *out_n, unsigned short *out_data) 
{
	int decoder_mask = (1<<he->decode_bits)-1;
	int current_bits = 0;
	int current = 0;
	int counted_bits = 0;
	int n=0;
	int in_n = (in_data_bits + 15) / 16;

	*out_n=0;

	while (1) {

		huffman_decode_table_16_t *entry; 

		while (current_bits < he->decode_bits) {

			if (n++ >= in_n) 
				break;

			current |= ((*in_data++) << current_bits);
			current_bits += 16;
		}

		entry = &he->decode_table[current & decoder_mask];

		current >>= entry->count;

		current_bits -= entry->count;

		out_data[(*out_n)++] = entry->value;

		if ( (counted_bits += entry->count) == in_data_bits) //have we counted all the bits?
			break;
	}
}

// #define TEST1
// #define TEST2
// #define TEST3
// #define TEST4
// #define TEST5
#define TEST6
 
// #define TESTING_MAIN
#ifdef TESTING_MAIN
int main()
{
	huffman_encoder_8_t * he_8 = new_huffman_encoder_8();
	huffman_encoder_16_t * he_16 = new_huffman_encoder_16();

	char out_data_8[4096];
	char out_data2_8[4096];
	unsigned short out_data_16[4096];
	unsigned short out_data2_16[4096];
	int out_len, out_len2;
	float compression_ratio_8, compression_ratio_16;

#ifdef TEST1
	int uncomp_add_len = (int)strlen("Hello World!")+1;
	int uncomp_enc_len = uncomp_add_len;

	char test_add_8[] = "abcc";
	char *test_enc_8 = &test_add_8[0];

	wchar_t test_add_16[] = L"abcc";
	wchar_t *test_enc_16 = &test_add_16[0];
#endif
#ifdef TEST2
	int uncomp_add_len = (int)strlen("Hello World!")+1;
	int uncomp_enc_len = uncomp_add_len;

	char test_add_8[] = "Hello World!";
	char *test_enc_8 = &test_add_8[0];

	wchar_t test_add_16[] = L"Hello World!";
	wchar_t *test_enc_16 = &test_add_16[0];
#endif
#ifdef TEST3
	int uncomp_add_len = (int)strlen("klj324908lfdnoi83q56 70ewrpi;978)(08p7$RW#\n$&*jilk.34jp807qj34%#@ 78ws9o8ul\nkm(8452joiojig809u3TE A3q49e8zj;o3q4TkijuIU:u98P*\n (8^&TGp97t5r4;4/6q3$tuiw4u8potK&\n&*LL*ILK8y%^E#4w543.998000 0reki89o.*IU.,i.kljY4e452ol0-[\"89-\n\n[(O&08- 7[PQ34reg87234\t wenoi9qa83o8u&*(hlu a&(*/l53kEAr7qeagzhq35ajeRT&9o6 jep95t\n3qw")+1;
	int uncomp_enc_len = uncomp_add_len;

	char test_add_8[] = "klj324908lfdnoi83q56 70ewrpi;978)(08p7$RW#\n$&*jilk.34jp807qj34%#@ 78ws9o8ul\nkm(8452joiojig809u3TE A3q49e8zj;o3q4TkijuIU:u98P*\n (8^&TGp97t5r4;4/6q3$tuiw4u8potK&\n&*LL*ILK8y%^E#4w543.998000 0reki89o.*IU.,i.kljY4e452ol0-[\"89-\n\n[(O&08- 7[PQ34reg87234\t wenoi9qa83o8u&*(hlu a&(*/l53kEAr7qeagzhq35ajeRT&9o6 jep95t\n3qw";
	char *test_enc_8 = &test_add_8[0];

	wchar_t test_add_16[] = L"klj324908lfdnoi83q56 70ewrpi;978)(08p7$RW#\n$&*jilk.34jp807qj34%#@ 78ws9o8ul\nkm(8452joiojig809u3TE A3q49e8zj;o3q4TkijuIU:u98P*\n (8^&TGp97t5r4;4/6q3$tuiw4u8potK&\n&*LL*ILK8y%^E#4w543.998000 0reki89o.*IU.,i.kljY4e452ol0-[\"89-\n\n[(O&08- 7[PQ34reg87234\t wenoi9qa83o8u&*(hlu a&(*/l53kEAr7qeagzhq35ajeRT&9o6 jep95t\n3qw";
	wchar_t *test_enc_16 = &test_add_16[0];
#endif
#ifdef TEST4
	int uncomp_add_len = (int)strlen("Hello World!")+1;
	int uncomp_enc_len = (int)strlen("Jello World!")+1;

	char test_add_8[] = "Hello World!";
	char test_enc_8[] = "Jello World!";

	wchar_t test_add_16[] = L"Hello World!";
	wchar_t test_enc_16[] = L"Jello World!";
#endif
#ifdef TEST5
#define DO3
	int uncomp_add_len = (int)strlen("abcdefghijklmnopqrstuvwxyz")+1;
	int uncomp_add_len2 = (int)strlen("ABCDEFGHIJKLMNOPQRSTUVWXYZ")+1;
	int uncomp_add_len3 = (int)strlen("0123456789. ")+1;
	int uncomp_enc_len = (int)strlen("I live in a giant bucket")+1;

	char test_add_8[] = "abcdefghijklmnopqrstuvwxyz";
	char test_add_82[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char test_add_83[] = "0123456789. ";
	char test_enc_8[] = "I live in a giant bucket.";

	wchar_t test_add_16[] = L"abcdefghijklmnopqrstuvwxyz";
	wchar_t test_add_162[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	wchar_t test_add_163[] = L"0123456789. ";
	wchar_t test_enc_16[] = L"I live in a giant bucket.";
#endif
#ifdef TEST6
	int uncomp_add_len = (int)strlen("i might be movin to montana soon")+1;
	int uncomp_enc_len = (int)strlen("team ovations mignon mint hobo")+1;

	char test_add_8[] = "i might be movin to montana soon";
	char test_enc_8[] = "team ovations mignon mint hobo";

	wchar_t test_add_16[] = L"i might be movin to montana soon";
	wchar_t test_enc_16[] = L"team ovations mignon mint hobo";
#endif

	//8-bit test
	huffman_add_data_8(he_8, uncomp_add_len, test_add_8);
#ifdef DO3
	huffman_add_data_8(he_8, uncomp_add_len2, test_add_82);
	huffman_add_data_8(he_8, uncomp_add_len3, test_add_83);
#endif
	huffman_create_tables_8(he_8);
	if ( huffman_encode_8(he_8, uncomp_enc_len, test_enc_8, &out_len, out_data_8) )
		huffman_decode_8(he_8, out_len, out_data_8, &out_len2, out_data2_8);

	compression_ratio_8 = ((float)((out_len + 7) /8)) / (float)uncomp_add_len;

	//16-bit test
	huffman_add_data_16(he_16, uncomp_add_len, (unsigned short *)test_add_16);
#ifdef DO3
	huffman_add_data_16(he_16, uncomp_add_len2, (unsigned short *)test_add_162);
	huffman_add_data_16(he_16, uncomp_add_len3, (unsigned short *)test_add_163);
#endif
	huffman_create_tables_16(he_16);
	if ( huffman_encode_16(he_16, uncomp_enc_len, (unsigned short *)test_enc_16, &out_len, out_data_16) )
		huffman_decode_16(he_16, out_len, out_data_16, &out_len2, out_data2_16);

	compression_ratio_16 = ((float)((out_len + 15) /16)) /(float)uncomp_add_len;
	
	delete_huffman_encoder_8(he_8);
	delete_huffman_encoder_16(he_16);

	return 1;
}
#endif









