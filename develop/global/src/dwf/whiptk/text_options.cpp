//  Copyright (c) 1996-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/whiptk/text_options.cpp 3     5/09/05 12:41a Evansg $


#include "whiptk/pch.h"

///////////////////////////////////////////////////////////////////////////
int WT_Text_Optioncode::option_id_from_optioncode()
{
    if (!strcmp((char const *)token(), "(Overscore"))
        m_option_id = Overscore_Option;
    else if (!strcmp((char const *)token(), "(Underscore"))
        m_option_id = Underscore_Option;
    else if (!strcmp((char const *)token(), "(Bounds"))
        m_option_id = Bounds_Option;
    else
        m_option_id = Unknown_Option;

    return m_option_id;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Text_Option_Overscore::object_id() const
{
    return WT_Object::Text_Option_Overscore_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Text_Option_Underscore::object_id() const
{
    return WT_Object::Text_Option_Underscore_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Text_Option_Bounds::object_id() const
{
    return WT_Object::Text_Option_Bounds_ID;
}

///////////////////////////////////////////////////////////////////////////
WT_Object::WT_ID WT_Text_Option_Reserved::object_id() const
{
    return WT_Object::Text_Option_Reserved_ID;
}

//========================================================================================
//========================================================================================
//========================================================================================
//========================================================================================

WT_Result  WT_Text_Option_Scoring::set( const tPositionVector& rVector )
{
    m_count = (WT_Unsigned_Integer16)rVector.size();

    if (m_positions != WD_Null )
    {
        delete []m_positions;
    }

    if (m_count)
    {
        m_positions = new WT_Unsigned_Integer16[m_count];
        if (!m_positions)
            return WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_positions[ii] = rVector[ii];
    }
    else
        m_positions = WD_Null;

	return WT_Result::Success;
}

WT_Result WT_Text_Option_Scoring::set(WT_Unsigned_Integer16 count, WT_Unsigned_Integer16 const * pos)
{
    m_count = count;

    if (m_positions != WD_Null )
    {
        delete []m_positions;
    }

    if (m_count)
    {
        m_positions = new WT_Unsigned_Integer16[m_count];
        if (!m_positions)
            return WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_positions[ii] = pos[ii];
    }
    else
        m_positions = WD_Null;

	return WT_Result::Success;
}

#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_Option_Scoring::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary output
        WD_CHECK (file.write_count(m_count + 1));
        for (int ii = 0; ii < m_count; ii++)
            WD_CHECK (file.write_count(m_positions[ii] + 1));
    }
    else
    {
        // ASCII output
        if (m_count)
        {
            WD_CHECK (file.write((WT_Byte) ' '));
            WD_CHECK (file.write(opcode()));
            WD_CHECK (file.write(" ("));
            WD_CHECK (file.write_ascii(m_count));
            WD_CHECK (file.write((WT_Byte) ' '));

            WD_CHECK (file.write_ascii(m_positions[0]));
            for (int ii = 1; ii < m_count; ii++)
            {
                WD_CHECK (file.write((WT_Byte) ','));
                WD_CHECK (file.write_ascii(m_positions[ii]));
            }

            WD_CHECK (file.write("))"));
        } // if (m_count)
    } // ascii

    return WT_Result::Success;
}
#else
WT_Result WT_Text_Option_Scoring::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Text_Option_Scoring::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        // Binary
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Count;

            // No break
        case Getting_Count:
            WT_Integer32 tmp;
            WD_CHECK (file.read_count (tmp));
            if (--tmp > 65535)
                return WT_Result::Corrupt_File_Error;

            m_count = static_cast<WT_Unsigned_Integer16>(tmp);

            delete []m_positions;
            m_positions = WD_Null;

            if (m_count)
            {
                m_positions = new WT_Unsigned_Integer16 [m_count];
                if (!m_positions)
                    return WT_Result::Out_Of_Memory_Error;
            }
            m_materialization_counter = 0;
            m_stage = Getting_Positions;

            // No break
        case Getting_Positions:
            while (m_materialization_counter < m_count)
            {
                WT_Integer32 tmp;
                WD_CHECK (file.read_count(tmp));
                m_positions[m_materialization_counter++] = static_cast<WT_Unsigned_Integer16>(tmp) - 1;
            }

            break;
        default:
            return WT_Result::Internal_Error;
        } // switch (m_stage)
    }
    else
    {
        // ASCII
        switch (m_stage)
        {
        case Starting:
            m_stage = Eating_Pre_Open_Paren_Whitespace;

            // No break
        case Eating_Pre_Open_Paren_Whitespace:
            WD_CHECK (file.eat_whitespace());
            m_stage = Getting_Open_Paren;

            // No break
        case Getting_Open_Paren:
            WT_Byte a_byte;

            WD_CHECK (file.read(a_byte));
            if (a_byte != '(')
                return WT_Result::Corrupt_File_Error;

            file.increment_paren_count();
            m_stage = Getting_Count;

            // No break
        case Getting_Count:
            WD_CHECK (file.read_ascii(m_count));

            delete []m_positions;
            m_positions = WD_Null;

            if (m_count)
            {
                m_positions = new WT_Unsigned_Integer16 [m_count];
                if (!m_positions)
                    return WT_Result::Out_Of_Memory_Error;
            }
            m_materialization_counter = 0;
            m_stage = Getting_Positions;

            // No break
        case Getting_Positions:
            while (m_materialization_counter < m_count)
            {
                WD_CHECK (file.read_ascii(m_positions[m_materialization_counter]));
                m_materialization_counter++;
                if (m_materialization_counter < m_count)
                {
                    // Eat the comma
                    WT_Byte a_byte;
                    WD_CHECK (file.read(a_byte));
                    if (a_byte != ',')
                        return WT_Result::Corrupt_File_Error;
                }
            }

            m_stage = Getting_Close_Parens;

            // No break
        case Getting_Close_Parens:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Complain ("unexpected stage during materialize");
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // ASCII

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Scoring::WT_Text_Option_Scoring(WT_Text_Option_Scoring const & scoring) throw(WT_Result)
  : WT_Option()
, m_count (scoring.count())
  , m_materialization_counter(0)
{
    m_stage = WT_Text_Option_Scoring::Starting;

    if (m_count)
    {
        m_positions = new WT_Unsigned_Integer16[m_count];
        if (!m_positions)
            throw WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_positions[ii] = scoring.positions()[ii];
    }
    else
        m_positions = WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Scoring const & WT_Text_Option_Scoring::operator= (WT_Text_Option_Scoring const & scoring) throw(WT_Result)
{
    delete []m_positions;
    m_positions = WD_Null;

    m_count = scoring.count();

    if (m_count)
    {
        m_positions = new WT_Unsigned_Integer16[m_count];
        if (!m_positions)
            throw WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_positions[ii] = scoring.positions()[ii];
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Scoring::~WT_Text_Option_Scoring()
{
    delete []m_positions;
}


//========================================================================================
//========================================================================================
//========================================================================================
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_Option_Bounds::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary output
        WD_Assert(m_bounds);
        if (!m_bounds)
            return WT_Result::Toolkit_Usage_Error;

        WD_CHECK (file.write(4, m_bounds));
    }
    else
    {
        // ASCII output
        if (m_bounds)
        {
            WD_CHECK (file.write(" (Bounds "));
            WD_CHECK (file.write_ascii(4, m_bounds));
            WD_CHECK (file.write((WT_Byte) ')'));
        } // if (m_bounds)
    } // ascii

    return WT_Result::Success;
}
#else
WT_Result WT_Text_Option_Bounds::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
WT_Result   WT_Text_Option_Bounds::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        // Binary
        if (!m_bounds)
        {
            m_bounds = new WT_Logical_Point[4];
            if (!m_bounds)
                return WT_Result::Out_Of_Memory_Error;
        }

        WD_CHECK (file.read(4, m_bounds));
    }
    else
    {
        // ASCII
        switch (m_stage)
        {
        case Getting_Operand:
            if (!m_bounds)
            {
                m_bounds = new WT_Logical_Point[4];
                if (!m_bounds)
                    return WT_Result::Out_Of_Memory_Error;
            }

            m_materialization_counter = 0;
            m_stage = Getting_Bounds;

        case Getting_Bounds:

            while (m_materialization_counter < 4)
            {
                WD_CHECK(file.read_ascii(m_bounds[m_materialization_counter]));
                m_materialization_counter++;
            }

            m_stage = Getting_Close_Paren;

            // No break
        case Getting_Close_Paren:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Assert ("unexpected stage during materialize");
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // ASCII

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_Option_Bounds::set(WT_Logical_Point const * bounds)
{
    delete []m_bounds;

    if (bounds)
    {
        m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            return WT_Result::Out_Of_Memory_Error;

        m_bounds[0] = bounds[0];
        m_bounds[1] = bounds[1];
        m_bounds[2] = bounds[2];
        m_bounds[3] = bounds[3];
    }
    else
        m_bounds = WD_Null;

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
void WT_Text_Option_Bounds::transform(WT_Transform const & transform)
{
    if (m_bounds)
    {
        WT_Logical_Point    tmp_pts[4];

        tmp_pts[0] = m_bounds[0];
        tmp_pts[1] = m_bounds[1];
        tmp_pts[2] = m_bounds[2];
        tmp_pts[3] = m_bounds[3];

        m_bounds[0] = tmp_pts[0] * transform;
        m_bounds[1] = tmp_pts[1] * transform;
        m_bounds[2] = tmp_pts[2] * transform;
        m_bounds[3] = tmp_pts[3] * transform;
    }
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
void WT_Text_Option_Bounds::relativize(WT_File & file)
{

    if (m_bounds)
    {
        WT_Logical_Point    tmp_pts[4];

        tmp_pts[0] = m_bounds[0];
        tmp_pts[1] = m_bounds[1];
        tmp_pts[2] = m_bounds[2];
        tmp_pts[3] = m_bounds[3];

        m_bounds[0] = file.update_current_point(tmp_pts[0]);
        m_bounds[1] = file.update_current_point(tmp_pts[1]);
        m_bounds[2] = file.update_current_point(tmp_pts[2]);
        m_bounds[3] = file.update_current_point(tmp_pts[3]);
    }
}
#else
void WT_Text_Option_Bounds::relativize(WT_File &)
{
}
#endif  // DESIRED_CODE()

///////////////////////////////////////////////////////////////////////////
void WT_Text_Option_Bounds::de_relativize(WT_File & file)
{
    if (m_bounds)
    {
        WT_Logical_Point    tmp_pts[4];

        tmp_pts[0] = m_bounds[0];
        tmp_pts[1] = m_bounds[1];
        tmp_pts[2] = m_bounds[2];
        tmp_pts[3] = m_bounds[3];

        m_bounds[0] = file.de_update_current_point(tmp_pts[0]);
        m_bounds[1] = file.de_update_current_point(tmp_pts[1]);
        m_bounds[2] = file.de_update_current_point(tmp_pts[2]);
        m_bounds[3] = file.de_update_current_point(tmp_pts[3]);
    }
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Bounds::WT_Text_Option_Bounds(WT_Text_Option_Bounds const & bounds) throw(WT_Result)
  : WT_Option()
  , m_materialization_counter(0)
{
    if (bounds.bounds())
    {
        m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            throw WT_Result::Out_Of_Memory_Error;

        m_bounds[0] = bounds.bounds()[0];
        m_bounds[1] = bounds.bounds()[1];
        m_bounds[2] = bounds.bounds()[2];
        m_bounds[3] = bounds.bounds()[3];
    }
    else
        m_bounds = WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Bounds const & WT_Text_Option_Bounds::operator= (WT_Text_Option_Bounds const & bounds) throw(WT_Result)
{
    delete []m_bounds;
    m_bounds = WD_Null;

    if (bounds.bounds())
    {
        m_bounds = new WT_Logical_Point[4];
        if (!m_bounds)
            throw WT_Result::Out_Of_Memory_Error;

        m_bounds[0] = bounds.bounds()[0];
        m_bounds[1] = bounds.bounds()[1];
        m_bounds[2] = bounds.bounds()[2];
        m_bounds[3] = bounds.bounds()[3];
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Bounds::~WT_Text_Option_Bounds()
{
    delete []m_bounds;
}

//========================================================================================
//========================================================================================
//========================================================================================
WT_Result WT_Text_Option_Reserved::set(WT_Unsigned_Integer16 count, WT_Unsigned_Integer16 const * values)
{
    m_count = count;

    delete []m_values;

    if (m_count)
    {
        m_values = new WT_Unsigned_Integer16[m_count];
        if (!m_values)
            return WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_values[ii] = values[ii];
    }
    else
        m_values = WD_Null;

	return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_Text_Option_Reserved::serialize(WT_Object const &, WT_File & file) const
{
    if (file.heuristics().allow_binary_data())
    {
        // Binary output
        WD_CHECK (file.write_count(m_count + 1));
        for (int ii = 0; ii < m_count; ii++)
            WD_CHECK (file.write_count(m_values[ii] + 1));
    }
    else
    {
        // ASCII output
        if (m_count)
        {
            WD_CHECK (file.write(" (CharPos ("));
            WD_CHECK (file.write_ascii(m_count));
            WD_CHECK (file.write((WT_Byte) ' '));

            WD_CHECK (file.write_ascii(m_values[0]));
            for (int ii = 1; ii < m_count; ii++)
            {
                WD_CHECK (file.write((WT_Byte) ','));
                WD_CHECK (file.write_ascii(m_values[ii]));
            }
            WD_CHECK (file.write("))"));
        } // if (m_count)
    } // ascii

    return WT_Result::Success;
}
#else
WT_Result WT_Text_Option_Reserved::serialize(WT_Object const &, WT_File &) const
{
    return WT_Result::Success;
}
#endif  // DESIRED_CODE()


///////////////////////////////////////////////////////////////////////////
WT_Result WT_Text_Option_Reserved::materialize(WT_Object &, WT_Optioncode const & optioncode, WT_File & file)
{
    if (optioncode.type() != WT_Opcode::Extended_ASCII)
    {
        // Binary
        switch (m_stage)
        {
        case Starting:
            m_stage = Getting_Count;

            // No break
        case Getting_Count:
            WT_Integer32 tmp;
            WD_CHECK (file.read_count (tmp));
            if (--tmp > 65535)
                return WT_Result::Corrupt_File_Error;

            m_count = static_cast<WT_Unsigned_Integer16>(tmp);

            delete []m_values;
            m_values = WD_Null;

            if (m_count)
            {
                m_values = new WT_Unsigned_Integer16 [m_count];
                if (!m_values)
                    return WT_Result::Out_Of_Memory_Error;
            }
            m_materialization_counter = 0;
            m_stage = Getting_Values;

            // No break
        case Getting_Values:
            while (m_materialization_counter < m_count)
            {
                WT_Integer32 tmp;
                WD_CHECK (file.read_count(tmp));
                m_values[m_materialization_counter++] = static_cast<WT_Unsigned_Integer16>(tmp) - 1;
            }

            break;
        default:
            return WT_Result::Internal_Error;
        } // switch (m_stage)
    }
    else
    {
        // ASCII
        switch (m_stage)
        {
        case Starting:
            m_stage = Eating_Pre_Open_Paren_Whitespace;

            // No break
        case Eating_Pre_Open_Paren_Whitespace:
            WD_CHECK (file.eat_whitespace());
            m_stage = Getting_Open_Paren;

            // No break
        case Getting_Open_Paren:
            {
            WT_Byte a_byte;

            WD_CHECK (file.read(a_byte));
            if (a_byte != '(')
                return WT_Result::Corrupt_File_Error;

            file.increment_paren_count();
            m_stage = Getting_Count;
            }
            // No break
        case Getting_Count:
            WD_CHECK (file.read_ascii(m_count));

            delete []m_values;
            m_values = WD_Null;

            if (m_count)
            {
                m_values = new WT_Unsigned_Integer16 [m_count];
                if (!m_values)
                    return WT_Result::Out_Of_Memory_Error;
            }
            m_materialization_counter = 0;
            m_stage = Getting_Values;

            // No break
        case Getting_Values:
            while (m_materialization_counter < m_count)
            {
                WD_CHECK (file.read_ascii(m_values[m_materialization_counter]));
                m_materialization_counter++;
                if (m_materialization_counter < m_count)
                {
                    // Eat the comma
                    WT_Byte a_byte;
                    WD_CHECK (file.read(a_byte));
                    if (a_byte != ',')
                        return WT_Result::Corrupt_File_Error;
                }
            }
            m_stage = Getting_Close_Parens;

            // No break
        case Getting_Close_Parens:

            WD_CHECK(optioncode.skip_past_matching_paren(file));
            m_stage = Completed;
            break;

        default:
            WD_Complain ("unexpected stage during materialize");
            return WT_Result::Internal_Error;
            
        } // switch (m_stage)
    } // ASCII

    m_materialized = WD_True;
    return WT_Result::Success;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Reserved::WT_Text_Option_Reserved(WT_Text_Option_Reserved const & reserved) throw(WT_Result)
  : WT_Option()
, m_count (reserved.count())
  , m_materialization_counter(0)
{
    m_stage = WT_Text_Option_Reserved::Starting;

    if (m_count)
    {
        m_values = new WT_Unsigned_Integer16[m_count];
        if (!m_values)
            throw WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_values[ii] = reserved.values()[ii];
    }
    else
        m_values = WD_Null;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Reserved const & WT_Text_Option_Reserved::operator= (WT_Text_Option_Reserved const & reserved) throw(WT_Result)
{
    delete []m_values;
    m_values = WD_Null;

    m_count = reserved.count();

    if (m_count)
    {
        m_values = new WT_Unsigned_Integer16[m_count];
        if (!m_values)
            throw WT_Result::Out_Of_Memory_Error;

        for (int ii = 0; ii < m_count; ii++)
            m_values[ii] = reserved.values()[ii];
    }

    return *this;
}

///////////////////////////////////////////////////////////////////////////
WT_Text_Option_Reserved::~WT_Text_Option_Reserved()
{
    delete[] m_values;
}
