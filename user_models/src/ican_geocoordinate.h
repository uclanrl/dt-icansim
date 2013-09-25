/*
* Copyright (c) 2013,  Network Research Lab, University of California, Los Angeles
* Coded by Yu-Ting Yu [yutingyu@cs.ucla.edu]
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
* in the documentation and/or other materials provided with the distribution.
* Neither the name of the University of California, Los Angeles nor the names of its contributors 
* may be used to endorse or promote products derived from this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ICAN_GEOCOORDINATE_H
#define ICAN_GEOCOORDINATE_H

typedef struct GeoCoordinate
{
    float x;
    float y;
    GeoCoordinate():x(-1.0), y(-1.0) {}
    GeoCoordinate(double ix, double iy):x(ix), y(iy) {}
    GeoCoordinate(GeoCoordinate const & o):x(o.x), y(o.y) {}
    ~GeoCoordinate() {}
    bool IsEmpty()
    {
        if(x==-1 && y==-1)
            return true;
        else return false;
    }

    bool operator<( const GeoCoordinate& n ) const
    {
        if(this->x < n.x)
            return true;
        else if(this->x == n.x)
        {
            if(this->y < n.y)
                return true;
            else
                return false;
        }
        else
            return false;
    }

    bool operator!=(const GeoCoordinate & n) const{
        if((this->x !=n.x) || (this->y!=n.y)) return true;
        else return false;
    }            
    
    bool operator==(const GeoCoordinate & n) const
    {
        return (this->x == n.x) && (this->y == n.y);
    }

    GeoCoordinate& operator =(GeoCoordinate const & o)
    {
        x = o.x;
        y = o.y;
        return *this;
    }

    void Print()
    {
        std::cout<<"("<<x<<", "<<y<<")";
    }

    std::string toString()
    {
        std::ostringstream s;
        s << "(" << x << ", " << y << ")";
        return s.str();
    }

    double GetDistance(float x2, float y2){
        return sqrt(pow((x2-x),2) + pow((y2-y),2));
    }

    double GetDistance( const GeoCoordinate& n ){
        return GetDistance(n.x, n.y);
    }

    bool Initialized(){
        if(x < 0 || y < 0) return false;
        else return true;
    }
} GeoCoordinate;


#endif
