/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, development version     *
*                (c) 2006-2017 INRIA, USTL, UJF, CNRS, MGH                    *
*                                                                             *
* This program is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This program is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this program. If not, see <http://www.gnu.org/licenses/>.        *
*******************************************************************************
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#define SOFA_HELPER_VECTOR_CPP
#include <sofa/helper/vector.h>
#include <sofa/helper/vector_device.h>
#include <sofa/helper/integer_id.h>
#include <sofa/helper/Factory.h>
#include <sofa/helper/BackTrace.h>
#include <cassert>
#include <iostream>

namespace sofa
{

namespace helper
{

#ifdef DEBUG_OUT_VECTOR
int cptid = 0;
#endif

void SOFA_HELPER_API vector_access_failure(const void* vec, unsigned size, unsigned i, const std::type_info& type)
{
    msg_error("vector") << "in vector<"<<gettypename(type)<<"> " << std::hex << (long)vec << std::dec << " size " << size << " : invalid index " << (int)i;
    BackTrace::dump();
    assert(i < size);
}

void SOFA_HELPER_API vector_access_failure(const void* vec, unsigned size, unsigned i, const std::type_info& type, const char* tindex)
{
    msg_error("vector") << "in vector<"<<gettypename(type)<<", integer_id<"<<tindex<<"> > " << std::hex << (long)vec << std::dec << " size " << size << " : invalid index " << (int)i;
    BackTrace::dump();
    assert(i < size);
}

/// Convert the string 's' into an unsigned int. The error are reported in msg & numErrors
/// is incremented.
int getInteger(const std::string& s, std::stringstream& msg, unsigned int& numErrors)
{
    const char* attrstr=s.c_str();
    char* end=nullptr;
    int retval = strtol(attrstr, &end, 10);

    /// It is important to check that the string was totally parsed to report
    /// message to users because a silent error is the worse thing that can happen in UX.
    if(end !=  attrstr+strlen(attrstr))
    {
        if(numErrors<5)
            msg << "    - problem while parsing '" << s <<"' as Integer'. Replaced by 0 instead." << msgendl ;
        if(numErrors==5)
            msg << "   - ... " << msgendl;
        numErrors++ ;
        return 0 ;
    }
    return retval ;
}

/// Convert the string 's' into an unsigned int. The error are reported in msg & numErrors
/// is incremented.
unsigned int getUnsignedInteger(const std::string& s, std::stringstream& msg, unsigned int& numErrors)
{
    const char* attrstr=s.c_str();
    char* end=nullptr;

    /// If there is minus sign we exit.
    if(s.find_first_of('-') != std::string::npos ){
        if(numErrors<5)
            msg << "   - problem while parsing '" << s <<"' as Unsigned Integer because the minus sign is not allowed'. Replaced by 0 instead." << msgendl ;
        if(numErrors==5)
            msg << "   - ... " << msgendl;
        numErrors++ ;
        return 0 ;
    }

    unsigned int retval = strtoll(attrstr, &end, 10);

    /// It is important to check that the string was totally parsed to report
    /// message to users because a silent error is the worse thing that can happen in UX.
    if(end !=  attrstr+strlen(attrstr))
    {
        if(numErrors<5)
            msg << "   - problem while parsing '" << s <<"' as Unsigned Integer'. Replaced by 0 instead." << msgendl ;
        if(numErrors==5)
            msg << "   - ... " << msgendl;
        numErrors++ ;
        return 0 ;
    }
    return retval ;
}


/// Input stream
/// Specialization for reading vectors of int and unsigned int using "A-B" notation for all integers between A and B, optionnally specifying a step using "A-B-step" notation.
//TODO(dmarchal 2017-05-13 (remove in 1 year if not done)): This code may have problem and requires a review.
// This implementation is broken !!
// -10--20 should return [-10 -11 -12 -13 -14 ....]
// -10--20-3 should rise a warning [-10 -11 -12 -13 -14 ....]
// -10--20--3 should work....
//
template<>
std::istream& SOFA_HELPER_API vector<int>::read( std::istream& in )
{
    int t;
    this->clear();
    std::string s;
    std::stringstream msg;
    unsigned int numErrors=0;

    /// Cut the input stream in words using the standard's '<space>' token eparator.
    while(in>>s)
    {
        /// Check if there is the sign '-' in the string s.
        std::string::size_type hyphen = s.find_first_of('-',1);

        /// If there is no '-' in s
        if (hyphen == std::string::npos)
        {
            /// Convert the word into an integer number.
            /// Use strtol because it reports error in case of parsing problem.
            t = getInteger(s, msg, numErrors) ;
            this->push_back(t);
        }

        /// If there is at least one '-'
        else
        {
            int t1,t2,tinc;
            std::string s1(s,0,hyphen);
            t1 = getInteger(s1, msg, numErrors) ;
            std::string::size_type hyphen2 = s.find_first_of('-',hyphen+2);
            if (hyphen2 == std::string::npos)
            {
                std::string s2(s,hyphen+1);
                t2 = getInteger(s2, msg, numErrors) ;
                tinc = (t1<t2) ? 1 : -1;
            }
            else
            {
                std::string s2(s,hyphen+1,hyphen2-hyphen-1);
                std::string s3(s,hyphen2+1);
                t2 =  getInteger(s2, msg, numErrors) ;
                tinc =  getInteger(s3, msg, numErrors) ;
                if (tinc == 0)
                {
                    tinc = (t1<t2) ? 1 : -1;
                    msg << "- Increment 0 is replaced by "<< tinc << msgendl;
                }
                if ((t2-t1)*tinc < 0)
                {
                    // increment not of the same sign as t2-t1 : swap t1<->t2
                    t = t1;
                    t1 = t2;
                    t2 = t;
                }
            }

            /// Go in backward order.
            if (tinc < 0)
                for (t=t1; t>=t2; t+=tinc)
                    this->push_back(t);
            /// Go in Forward order
            else
                for (t=t1; t<=t2; t+=tinc)
                    this->push_back(t);
        }
    }
    if( in.rdstate() & std::ios_base::eofbit ) { in.clear(); }
    if(numErrors!=0)
    {
        msg_warning("vector<int>") << "Unable to parse vector values:" << msgendl
                                   << msg.str() ;
    }
    return in;
}


/// Input stream
/// Specialization for reading vectors of int and unsigned int using "A-B" notation for all integers between A and B
template<>
std::istream& SOFA_HELPER_API vector<unsigned int>::read( std::istream& in )
{
    std::stringstream errmsg ;
    unsigned int errcnt = 0 ;
    unsigned int t = 0 ;

    this->clear();
    std::string s;
    while(in>>s)
    {
        std::string::size_type hyphen = s.find_first_of('-',1);
        if (hyphen == std::string::npos)
        {
            t = getUnsignedInteger(s, errmsg, errcnt) ;
            this->push_back(t);
        }
        else
        {
            unsigned int t1,t2;
            int tinc;
            std::string s1(s,0,hyphen);
            t1 = getUnsignedInteger(s1, errmsg, errcnt) ;
            std::string::size_type hyphen2 = s.find_first_of('-',hyphen+2);
            if (hyphen2 == std::string::npos)
            {
                std::string s2(s,hyphen+1);
                t2 = getUnsignedInteger(s2, errmsg, errcnt);
                tinc = (t1<=t2) ? 1 : -1;
            }
            else
            {
                std::string s2(s,hyphen+1,hyphen2-hyphen-1);
                std::string s3(s,hyphen2+1);
                t2 = getUnsignedInteger(s2, errmsg, errcnt);
                tinc = getInteger(s3, errmsg, errcnt);
                if (tinc == 0)
                {
                    tinc = (t1<=t2) ? 1 : -1;
                    errmsg << "- problem while parsing '"<<s<<"': increment is 0. Use " << tinc << " instead." ;
                }
                if (((int)(t2-t1))*tinc < 0)
                {
                    /// increment not of the same sign as t2-t1 : swap t1<->t2
                    t = t1;
                    t1 = t2;
                    t2 = t;
                }
            }
            if (tinc < 0){
                for (t=t1; t>t2; t=t+tinc)
                    this->push_back(t);
                this->push_back(t2);
            } else {
                for (t=t1; t<=t2; t=t+tinc)
                    this->push_back(t);
            }
        }
    }
    if( in.rdstate() & std::ios_base::eofbit ) { in.clear(); }
    if(errcnt!=0)
    {
        msg_warning("vector<unsigned int>") << "Unable to parse values" << msgendl
                                            << errmsg.str() ;
    }

    return in;
}

/// Output stream
/// Specialization for writing vectors of unsigned char
template<>
std::ostream& SOFA_HELPER_API vector<unsigned char>::write(std::ostream& os) const
{
    if( this->size()>0 )
    {
        for( size_type i=0; i<this->size()-1; ++i )
            os<<(int)(*this)[i]<<" ";
        os<<(int)(*this)[this->size()-1];
    }
    return os;
}

/// Input stream
/// Specialization for reading vectors of unsigned char
template<>
std::istream& SOFA_HELPER_API vector<unsigned char>::read(std::istream& in)
{
    int t;
    this->clear();
    while(in>>t)
    {
        this->push_back((unsigned char)t);
    }
    if( in.rdstate() & std::ios_base::eofbit ) { in.clear(); }
    return in;
}

} // namespace helper
} // namespace sofa
