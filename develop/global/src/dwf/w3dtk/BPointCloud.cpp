//
// Copyright (c) 2000 by Tech Soft 3D, LLC.
// The information contained herein is confidential and proprietary to
// Tech Soft 3D, LLC., and considered a trade secret as defined under
// civil and criminal statutes.  Tech Soft 3D shall pursue its civil
// and criminal remedies in the event of unauthorized use or misappropriation
// of its trade secrets.  Use of this information by anyone other than
// authorized employees of Tech Soft 3D, LLC. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
// $Header: //DWF/Working_Area/Willie.Zhu/w3dtk/BPointCloud.cpp#1 $
//
#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BStreamFileToolkit.h"
#include "dwf/w3dtk/BPointCloud.h"

#ifdef USE_BPOINTCLOUD

#include "dwf/w3dtk/BBytestream.h"
#include "dwf/w3dtk/BInternal.h"

#define ASSERT(x)

class BKDTree {
  protected:
    char m_partition_type;
    BKDTree *m_kids[2];
    int m_point_count;

    BKDTree( char partition_type ) { 
        m_partition_type = partition_type;
        m_kids[0] = m_kids[1] = null;
        m_point_count = 0;
    };

    BKDTree( const BKDTree &old ) { 
        m_partition_type = old.m_partition_type;
        m_kids[0] = old.m_kids[0];
        m_kids[1] = old.m_kids[1];
        m_point_count = old.m_point_count;
    };

    virtual ~BKDTree() {
        delete m_kids[0];
        delete m_kids[1];
    };

    TK_Status Insert( float *bbox, float const *pt, int max_depth, bool *unique, int index );
    void ResolveIDs( int *next_id, int max_depth, int *point_map );
    void Search( int *best_fit_id, float best_fit );

    friend class BPointCloud;
};


class BKDTreeLeaf : public BKDTree {
  protected:
    int m_id;           
    int *m_indices;     
    int m_indices_used;
    int m_indices_allocated;

    BKDTreeLeaf() : 
        BKDTree( 'l' ) 
    {
        m_id = -1;                  
        m_indices_used = 0;
        m_indices_allocated = 1;
        m_indices = new int[ m_indices_allocated ];
    }
    virtual ~BKDTreeLeaf() {
        delete [] m_indices;
    };
    TK_Status Insert( bool alter *unique, int index );
    friend class BKDTree;
};



void BKDTree::ResolveIDs( int *next_id, int max_depth, int *point_map ) {
    if( max_depth == 0 ) {
        int i;

        ASSERT( m_partition_type == 'l' );
        if( point_map ) {
            BKDTreeLeaf *leaf;
            leaf = (BKDTreeLeaf *)this;
            for( i = 0 ; i < leaf->m_indices_used ; i++ )
                point_map[ leaf->m_indices[ i ] ] = *next_id;
        }
        *next_id += m_point_count;
    }
    else {
        if( m_kids[ 0 ] != null ) {
            m_kids[ 0 ]->ResolveIDs( next_id, max_depth - 1, point_map );
        }
        if( m_kids[ 1 ] != null ) {
            m_kids[ 1 ]->ResolveIDs( next_id, max_depth - 1, point_map );
        }
    }
}


TK_Status BKDTreeLeaf::Insert( bool alter *unique, int index ) {
    if( m_point_count != 0 ) {
        *unique = false;
    }
    else {
        m_point_count++;
        *unique = true;
    }
    if( m_indices_used >= m_indices_allocated ) {
        int *temp;

        m_indices_allocated *= 2;
        temp = new int[ m_indices_allocated ];
        memcpy( temp, m_indices, m_indices_used * sizeof( int ) );
        delete [] m_indices;
        m_indices = temp;
    }
    m_indices[ m_indices_used++ ] = index;
    return TK_Normal;
}



TK_Status BKDTree::Insert ( float *bbox, float const *pt, int max_depth, bool alter *unique, int index ) {
    float cutoff;
    float bbox_save;
    TK_Status status = TK_Normal;
    char next_partition;

    switch( m_partition_type ) {
        case 'x': {
            next_partition = 'y';
            cutoff = bbox[0] + 0.5f * (bbox[3] - bbox[0]);
            if( pt[0] < cutoff ) {
                if( max_depth == 1 ) {
                    BKDTreeLeaf *leaf = new BKDTreeLeaf( );
                    m_kids[ 0 ] = leaf;
                    if( !m_kids[ 0 ] )
                        return TK_Error;
                    leaf->Insert( unique, index );
                }
                else {
                    if( m_kids[ 0 ] == null ) {
                        m_kids[ 0 ] = new BKDTree( next_partition );
                        if( !m_kids[ 0 ] )
                            return TK_Error;
                    }
                    bbox_save = bbox[3];
                    bbox[3] = cutoff;
                    status = m_kids[0]->Insert( bbox, pt, max_depth - 1, unique, index );
                    if( *unique )
                        m_point_count++;
                    bbox[3] = bbox_save;
                }
            } else  {
                if( max_depth == 1 ) {
                    BKDTreeLeaf *leaf = new BKDTreeLeaf( );
                    m_kids[ 1 ] = leaf;
                    if( !m_kids[ 1 ] )
                        return TK_Error;
                    leaf->Insert( unique, index );
                }
                else {
                    if( m_kids[ 1 ] == null ) {
                        m_kids[ 1 ] = new BKDTree( next_partition );
                        if( !m_kids[ 1 ] )
                            return TK_Error;
                    }
                    bbox_save = bbox[0];
                    bbox[0] = cutoff;
                    status = m_kids[1]->Insert( bbox, pt, max_depth - 1, unique, index );
                    if( *unique )
                        m_point_count++;
                    bbox[0] = bbox_save;
                }
            }
        } break;        
        case 'y': {
            next_partition = 'z';
            cutoff = bbox[1] + 0.5f * (bbox[4] - bbox[1]);
            if( pt[1] < cutoff ) {
                if( max_depth == 1 ) {
                    BKDTreeLeaf *leaf = new BKDTreeLeaf( );
                    m_kids[ 0 ] = leaf;
                    if( !m_kids[ 0 ] )
                        return TK_Error;
                    leaf->Insert( unique, index );
                }
                else {
                    if( m_kids[ 0 ] == null ) {
                        m_kids[ 0 ] = new BKDTree( next_partition );
                        if( !m_kids[ 0 ] )
                            return TK_Error;
                    }
                    bbox_save = bbox[4];
                    bbox[4] = cutoff;
                    status = m_kids[0]->Insert( bbox, pt, max_depth - 1, unique, index );
                    if( *unique )
                        m_point_count++;
                    bbox[4] = bbox_save;
                }
            } else  {
                if( max_depth == 1 ) {
                    BKDTreeLeaf *leaf = new BKDTreeLeaf( );
                    m_kids[ 1 ] = leaf;
                    if( !m_kids[ 1 ] )
                        return TK_Error;
                    leaf->Insert( unique, index );
                }
                else {
                    if( m_kids[ 1 ] == null ) {
                        m_kids[ 1 ] = new BKDTree( next_partition );
                        if( !m_kids[ 1 ] )
                            return TK_Error;
                    }
                    bbox_save = bbox[1];
                    bbox[1] = cutoff;
                    status = m_kids[1]->Insert( bbox, pt, max_depth - 1, unique, index );
                    if( *unique )
                        m_point_count++;
                    bbox[1] = bbox_save;
                }
            }
        } break;
        case 'z': {
            next_partition = 'x';
            cutoff = bbox[2] + 0.5f * (bbox[5] - bbox[2]);
            if( pt[2] < cutoff ) {
                if( max_depth == 1 ) {
                    BKDTreeLeaf *leaf = new BKDTreeLeaf( );
                    m_kids[ 0 ] = leaf;
                    if( !m_kids[ 0 ] )
                        return TK_Error;
                    leaf->Insert( unique, index );
                }
                else {
                    if( m_kids[ 0 ] == null ) {
                        m_kids[ 0 ] = new BKDTree( next_partition );
                        if( !m_kids[ 0 ] )
                            return TK_Error;
                    }
                    bbox_save = bbox[5];
                    bbox[5] = cutoff;
                    status = m_kids[0]->Insert( bbox, pt, max_depth - 1, unique, index );
                    if( *unique )
                        m_point_count++;
                    bbox[5] = bbox_save;
                }
            } else  {
                if( max_depth == 1 ) {
                    BKDTreeLeaf *leaf = new BKDTreeLeaf( );
                    m_kids[ 1 ] = leaf;
                    if( !m_kids[ 1 ] )
                        return TK_Error;
                    leaf->Insert( unique, index );
                }
                else {
                    if( m_kids[ 1 ] == null ) {
                        m_kids[ 1 ] = new BKDTree( next_partition );
                        if( !m_kids[ 1 ] )
                            return TK_Error;
                    }
                    bbox_save = bbox[2];
                    bbox[2] = cutoff;
                    status = m_kids[1]->Insert( bbox, pt, max_depth - 1, unique, index );
                    if( *unique )
                        m_point_count++;
                    bbox[2] = bbox_save;
                }
            }
        } break;
    }
    return status;

} 


BPointCloud::BPointCloud( BStreamFileToolkit *tk ) {
    m_kd = null;
    m_resolution = 0;
    m_max_val = 0;
    m_cell_size[0] = m_cell_size[1] = m_cell_size[2] = 0;
    m_bbox[0] = m_bbox[1] = m_bbox[2] = m_bbox[3] = m_bbox[4] = m_bbox[5] = 0;
    m_point_count = 0;
    m_points = null;
    m_point_map = null;
    m_next_id = 0;
    m_tk = tk;
};


TK_Status BPointCloud::InitWrite( int resolution, const float *bbox, int original_point_count, const float *points  ) {
    int i;
    bool unique;
    TK_Status status = TK_Normal;

    m_kd = new BKDTree( 'x' );
    if( m_kd == null )
        return m_tk->Error( "memory allocation failure in BPointCloud::InitRead" );
    m_resolution = resolution;
    m_resolution -= (resolution % 3); 
    m_max_val = (0x1 << (resolution/3)) - 1;
    memcpy( m_bbox, bbox, 6 * sizeof( float ) );
    m_cell_size[0] = (m_bbox[3] - m_bbox[0]) / m_max_val;
    m_cell_size[1] = (m_bbox[4] - m_bbox[1]) / m_max_val;
    m_cell_size[2] = (m_bbox[5] - m_bbox[2]) / m_max_val;
    m_original_point_count = original_point_count;
    m_point_count = 0;
    m_next_id = 0;
    m_point_map = new int[ original_point_count ];
    for( i = 0 ; i < original_point_count ; i++ ) {
        m_point_map[ i ] = -1;
        status = m_kd->Insert( m_bbox, &points[ i*3 ], m_resolution, &unique, i );
        if( status == TK_Error )
            return m_tk->Error( "internal error or memory allocation failure during buildup of kd tree" );
        if( unique )
            m_point_count++;
    }
    m_kd->ResolveIDs( &m_next_id, resolution, m_point_map );
    return TK_Normal;
};


TK_Status BPointCloud::InitRead( int resolution, const float *bbox, int original_point_count ) {
    m_kd = new BKDTree( 'x' );
    if( m_kd == null )
        return m_tk->Error( "memory allocation failure in BPointCloud::InitRead" );
    m_resolution = resolution;
    m_resolution -= (resolution % 3); 
    m_max_val = (0x1 << (resolution/3)) - 1;
    memcpy( m_bbox, bbox, 6 * sizeof( float ) );
    m_cell_size[0] = (m_bbox[3] - m_bbox[0]) / m_max_val;
    m_cell_size[1] = (m_bbox[4] - m_bbox[1]) / m_max_val;
    m_cell_size[2] = (m_bbox[5] - m_bbox[2]) / m_max_val;
    m_original_point_count = original_point_count;
    m_point_count = 0;
    m_points = null;
    m_point_map = null;
    m_next_id = 0;
    return TK_Normal;
};


BPointCloud::~BPointCloud() {
    delete m_kd;
    delete [] m_points;
    delete [] m_point_map;
};




void BPointCloud::put( class BKDTree *b, class BPack *pack, int num_bits ) 
{
    while(((0x1 << num_bits) > m_point_count))
        num_bits--;
    if( b->m_kids[0] != null ) {
        pack->Put( b->m_kids[0]->m_point_count, num_bits );
        put( b->m_kids[0], pack, num_bits );
    }
    if( b->m_kids[1] != null ) {
        put( b->m_kids[1], pack, num_bits );
    }
}



TK_Status BPointCloud::get( BKDTree *b, BPack *pack, int num_bits, int i, int j, int k, int span ) 
{
    int count;
    char next_partition_type;

    if( span == 0 ) {
        float *pt = &m_points[ m_next_id * 3 ];
        if( i == m_max_val )
            pt[0] = m_bbox[3]; 
        else
            pt[0] = m_bbox[0] + i * m_cell_size[0];
        if( j == m_max_val )
            pt[1] = m_bbox[4]; 
        else
            pt[1] = m_bbox[1] + j * m_cell_size[1];
        if( k == m_max_val )
            pt[2] = m_bbox[5]; 
        else
            pt[2] = m_bbox[2] + k * m_cell_size[2];
        m_next_id++;
        if( m_next_id >= m_point_count )
            return TK_Error;
        return TK_Normal;
    }

    while(((0x1 << num_bits) > m_point_count))
        num_bits--;
    count = pack->Get( num_bits );

    switch( b->m_partition_type ) {
        case 'x': {
            next_partition_type = 'y';
            if( count != 0 ) {
                b->m_kids[0] = new BKDTree( next_partition_type );
                b->m_kids[0]->m_point_count = count;
                get( b->m_kids[0], pack, num_bits, i, j, k, span );
            }
            if( count != m_point_count ) {
                b->m_kids[1] = new BKDTree( next_partition_type );
                b->m_kids[1]->m_point_count = m_point_count - count;
                get( b->m_kids[1], pack, num_bits, i+span, j, k, span );
            }
        } break;
        case 'y': {
            next_partition_type = 'z';
            if( count != 0 ) {
                b->m_kids[0] = new BKDTree( next_partition_type );
                b->m_kids[0]->m_point_count = count;
                get( b->m_kids[0], pack, num_bits, i, j, k, span );
            }
            if( count != m_point_count ) {
                b->m_kids[1] = new BKDTree( next_partition_type );
                b->m_kids[1]->m_point_count = m_point_count - count;
                get( b->m_kids[1], pack, num_bits, i, j+span, k, span );
            }
        } break;
        case 'z': {
            next_partition_type = 'x';
            if( count != 0 ) {
                b->m_kids[0] = new BKDTree( next_partition_type );
                b->m_kids[0]->m_point_count = count;
                get( b->m_kids[0], pack, num_bits, i, j, k, span>>1 ); 
            }
            if( count != m_point_count ) {
                b->m_kids[1] = new BKDTree( next_partition_type );
                b->m_kids[1]->m_point_count = m_point_count - count;
                get( b->m_kids[1], pack, num_bits, i, j, k+span, span>>1 ); 
            }
        } break;
        default:
            return TK_Error;
    }
    return TK_Normal;

} 


int BPointCloud::GetSizeUpperBound() const
{ 
    int log2n = 0;
    while( (0x1<<log2n) < m_point_count )
        log2n++;

    return 
        1 +                             
        4 +                             
        log2n * m_point_count +         
        log2n * m_original_point_count; 
}


void BPointCloud::Generate( unsigned char *buffer, int bytes_allocated, int *bytes_used )
{
    int i;
    int log2n;
    BPack pack;

    pack.InitWrite( bytes_allocated, buffer );
    pack.Put( 32, m_point_count );
    log2n = 0;
    while( (0x1<<log2n) < m_point_count )
        log2n++;
    put( m_kd, &pack, log2n );
    for( i = 0 ; i < m_original_point_count ; i++ ) {
        pack.Put( log2n, m_point_map[ i ] );
    }
    pack.SwapBytes(); 
    if( bytes_used != null )
        *bytes_used = pack.NumBytes();
}


TK_Status BPointCloud::Parse( const unsigned char *buffer, int size )
{
    int i, log2n, bits_per_sample, span;
    BPack pack;

    pack.InitRead( size, buffer );
    pack.SwapBytes(); 
    m_point_count = pack.Get( 32 );
    if( m_point_count <= 0 )
        return TK_Error;
    m_points = new float[ 3 * m_point_count ];
    if( m_points == null )
        return TK_Error;
    log2n = 0;
    while( (0x1<<log2n) < m_point_count )
        log2n++;
    bits_per_sample = m_resolution/3;
    span = 0x1 << (bits_per_sample - 1);
    get( m_kd, &pack, log2n, 0, 0, 0, span );
    for( i = 0 ; i < m_original_point_count ; i++ ) {
        m_point_map[ i ] = pack.Get( log2n );
    }
    return TK_Normal;
}



TK_Status BPointCloud::GetPoints( float *points_out ) 
{
    int i;
    float *src, *dest;

    for( i = 0 ; i < m_original_point_count ; i++ ) {
        if( m_point_map[i] < 0 || m_point_map[i] >= m_point_count )
            return m_tk->Error( "internal error: point map out of range in BPointCloud" );
        src = &m_points[ m_point_map[ i ] * 3 ];
        dest = &points_out[ i * 3 ];
        dest[0] = src[0];
        dest[1] = src[1];
        dest[2] = src[2];
    }
    return TK_Normal;
}

#endif 


