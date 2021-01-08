#ifndef PI_H
#define PI_H

class pi{

  private: //internal members //member variables
  float kp, ki, T;
  float k1, k2, e, p, ep, yp;
  static float ip;
  static float i;
  float dead_zone(float, float=-0.01, float=0.01);
  
  public: 
  pi(float kp=3, float ki=0.1, float T=10);
  float calc( float=0, float=0 );
  float anti_wdup(bool, bool, float);
};

float pi::ip=0;
float pi::i=0;


pi::pi(float p, float i, float T ): 
T(T), kp (p), ki (i), ep (0), k1 (ki*T/2), e(0), p(0)
{}

float pi::calc( float ref, float y ) {
  e = ref - y;
  p = kp*e;
  i = ip + k1 * ( e + ep ); 
  yp = y; ip = i;  ep = e;
return p+i; 
}

float pi::anti_wdup( bool over, bool under,float ff){
  if(over and !under) {
    i= 255-ff-p;
    ip=i;
    return p+i;
  
  }
  if(under and !over) {
    i=0-ff-p;
    ip=i;
    return p+i;
  }
}
float pi::dead_zone(float x, float xmin, float xmax){
  if(x>=xmax){ 
    return x-xmax;
  }else if(x<=xmin){
    return x+xmin;
  }else{
    return 0;
  }
}
#endif //PI_H
