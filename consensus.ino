#include <math.h>

enum States1 {INITIALISATION,READY_CONSENSUS, ITERATIONS, EXCHANGE, READY_EX, AVERAGE, LAGRANGIAN, FINAL};
States1 cs_solver=INITIALISATION;

void distributed_solver(){
  

  switch(cs_solver){
  
  case INITIALISATION:
  canWrite(0,12);//tell everybody I'm ready
  a=1;
  cs_solver=READY_CONSENSUS;
  break;

  case READY_CONSENSUS://wait until everybody has said they are ready with their disturbance;
       if(Ready==nodesCont.numberOfNodes()){
        Ready=0;
        cs_solver=ITERATIONS;
        }
  break;
  //iterations
   //COMPUTATION OF THE PRIMAL SOLUTIONS
   case ITERATIONS:
   if(partial_solution_ready==0){
   consensus_iterate(rho);//takes 8 loops max -> changes d
   }else if(partial_solution_ready==1){
    partial_solution_ready=0;
    for(uint8_t counter=0;counter<=nodesCont.numberOfNodes();counter++){
      d_all[counter][node.myHwId-1]=d[counter];
    }
     cs_solver=EXCHANGE;
   }
   break;
   
   ////
   // Nodes exchange their solutions
   case EXCHANGE:
    for(uint8_t counter=0;counter<=nodesCont.numberOfNodes();counter++){
      if(d[counter]<=0)
      canWrite(0,9+counter,0,0);//send solutions to everyone
      else
      canWrite(0,9+counter,0,d[counter]);//send solutions to everyone

    }
    canWrite(node.myHwId,12);//tell everyone I'm ready to proceed
    cs_solver= AVERAGE;
   break;

   //WAIT UNTIL EVERYBODY IS READY
    case READY_EX:
    if(Ready==nodesCont.numberOfNodes()){
      Ready=0;
      cs_solver=AVERAGE;
    }
    break;
    
   ////
   //COMPUTATION OF THE AVERAGE
   case AVERAGE:
   Serial.println("SOLUTION OF THE OTHER ARDUINO");
   for( uint8_t counter=0; counter<3; counter++){//iterate on the colums
        Serial.println(d_all[counter][nodesCont.getNextNode()-1]);
        d_av[counter]=0;
   }
   
   Serial.println("Average solution:");
    for( uint8_t counter2=0; counter2<3; counter2++){//iterate on the rows
      for( uint8_t counter=0; counter<3; counter++){//iterate on the colums
        d_av[counter2] += d_all[counter2][counter];//add the solutions for one light of every arduino 
      }
      d_av[counter2]=d_av[counter2]/(nodesCont.numberOfNodes()+1); //compute average
      Serial.println(d_av[counter2]);
    }
     cs_solver= LAGRANGIAN;
   break;
   ////
   //COMPUTATION OF THE LAGRANGIAN UPDATES
   case LAGRANGIAN:
   for( uint8_t counter=0; counter<=2; counter++){
     y[counter] = y[counter] +  rho*(d[counter]-d_av[counter]);
   }
     if(a<maxiter){
      Serial.print("ITERATION N°: ");
      Serial.println(a);
      ++a;  
      cs_solver= ITERATIONS;
     }
     else cs_solver= FINAL;
   break;

   case FINAL:
    solution_ready=1;
    percent=d[node.myHwId-1];
    cs_solver=INITIALISATION;
    break;
  } 
}



//FUNCTION TO COMPUTE THE AUGMENTED LAGRANGIAN COST AT A POSSIBLE SOLUTaON
//USED BY CONSENSUS_ITERATE
float evaluate_cost(float d[3],float rho){
     float result;
     for(uint8_t counter=0; counter<3; counter++){
     result+= c[counter]*d[counter] +y[counter]*(d[counter]-d_av[counter]);
     }
     return result + rho/2*pow(norm(d,d_av),2);
}

float norm( float d[3], float d_av[3]){
  return sqrt(pow(d[0]-d_av[0],2)+pow(d[1]-d_av[1],2)+pow(d[2]-d_av[2],2));
}

//FUNCTION TO CHECK SOLUTION FEASIBILITY
bool check_feasibility(float d[3]){
   const float tol = 0.001; ////tolerance for rounding errors
   if (d[node.myHwId-1] < 0-tol) {
    return 0; 
   }
   if (d[node.myHwId-1] > 100+tol) {
    return 0;
   }
   float vector_product=0;
   for(uint8_t counter=0; counter<nodesCont.numberOfNodes()+1; counter++){
    vector_product+=d[counter]*node.k[counter];
   }
  if (vector_product< node.L[node.myHwId-1]-node.o-tol) {
    return 0;
  }
   
   return 1;
}

enum States2 {SET, UNCONSTRAINED, CONST_LIN, CONST_0, CONST_100, CONST_LIN_0, CONST_LIN_100, END};
States2 cs_consensus= SET;


//FUNCTION TO COMPUTE THE PRIMAL SOLUTION
void consensus_iterate(float rho) {
 const float n = pow(sqrt(pow(node.k[0],2)+pow(node.k[1],2)+pow(node.k[2],2)),2); 
 const float m2 = n-pow(node.k[node.myHwId-1],2);
  switch(cs_consensus){

    case SET:
      
      c[node.myHwId-1]=node.c;
      /*d_best[0] = -1;
      d_best[1]=-1;
      d_best[2]=-1;*/
      cost_best = 1000000; //large number
      sol = 0;
      cost = 1;
      for( uint8_t counter=0; counter<3; counter++){
        z[counter]=rho*d_av[counter] - y[counter] - c[counter];
      }
      w=z[0]*node.k[0]+z[1]*node.k[1]+z[2]*node.k[2];
      cs_consensus= UNCONSTRAINED;
    
    break;
    
    case UNCONSTRAINED:
    //unconstrained minimum
      {
        //Serial.println("UNCONSTRAINED SOLUTION");
          float d_u[3] ={};
        for(uint8_t counter=0; counter<3; counter++){
          d_u[counter]=(1/rho)*z[counter];
          //Serial.println(d_u[counter]);
        }
        sol = check_feasibility(d_u);
       /* Serial.print("FEASIBILITY: ");
        Serial.println(sol);*/
        if(sol){
            //REVISE: IF UNCONSTRAINED SOLUTION EXISTS, THEN IT IS OPTIMAL
            //NO NEED TO COMPUTE THE OTHER
            cost = evaluate_cost(d_u, rho);
            if (cost < cost_best){
               for(uint8_t counter=0; counter<3; counter++){
               d_best[counter] = d_u[counter];
               }
               cost_best = cost;
            }
            cs_consensus=END;
         }
        else cs_consensus=CONST_LIN;
      }
     break;
     
    //compute minimum constrained to linear boundary   
    case CONST_LIN:
    {
      //Serial.println("CONST LIN SOLUTION");
      float d_bl[3] ={};
      for(uint8_t counter=0; counter<3; counter++){
        d_bl[counter]=(1/rho)*z[counter] - node.k[counter]/n*(node.o-node.L[node.myHwId-1]+(1/rho)*w);
        //Serial.println(d_bl[counter]);
      }
      //check feasibility of minimum constrained to linear boundary
      sol = check_feasibility(d_bl);
        /*Serial.print("FEASIBILITY: ");
        Serial.println(sol);*/
      // compute cost and if best store new optimum
      if (sol) {
          cost = evaluate_cost(d_bl, rho);
          if (cost < cost_best){
             for(uint8_t counter=0; counter<3; counter++){
              d_best[counter] = d_bl[counter];
             }
             cost_best = cost;
           }
      }
      cs_consensus= CONST_0;
    }
     break;
     
    //compute minimum constrained to 0 boundary
    case CONST_0:
    {
      //Serial.println("CONST 0 SOLUTION");
      float d_b0[3] = {};
      for(uint8_t counter=0; counter<3; counter++){
        d_b0[counter]=(1/rho)*z[counter];
        //Serial.println(d_b0[counter]);
      }
      d_b0[node.myHwId-1] = 0;
      //check feasibility of minimum constrained to 0 boundary
      sol = check_feasibility(d_b0);
        /*Serial.print("FEASIBILITY: ");
        Serial.println(sol);*/
      // compute cost and if best store new optimum
      if (sol){
          cost = evaluate_cost( d_b0, rho);
          if (cost < cost_best){
             for(uint8_t counter=0; counter<3; counter++){
              d_best[counter] = d_b0[counter];
             }
             cost_best = cost;
          }
      }
      cs_consensus=CONST_100;
    }
    break;
    
    //compute minimum constrained to 100 boundary
    case CONST_100:
    {
      //Serial.println("CONST 100 SOLUTION");
      float d_b1[3] = {};
      for(uint8_t counter=0; counter<3; counter++){
              d_b1[counter] = (1/rho)*z[counter];
              //Serial.println(d_b1[counter]);
             }
      d_b1[node.myHwId-1] = 100;
      //check feasibility of minimum constrained to 100 boundary
      sol = check_feasibility( d_b1);
        /*Serial.print("FEASIBILITY: ");
        Serial.println(sol);*/
      // compute cost and if best store new optimum
      if (sol){ 
          cost = evaluate_cost( d_b1, rho);
          if (cost < cost_best){
             for(uint8_t counter=0; counter<3; counter++){
              d_best[counter] = d_b1[counter];
             }
             cost_best = cost;
          }
      }
      cs_consensus=CONST_LIN_0;
    }
     break;
     
    // compute minimum constrained to linear and 0 boundary
    case CONST_LIN_0:
    {
      //Serial.println("CONST LIN 0 SOLUTION");
      float d_l0[3] = {};
      for(uint8_t counter=0; counter<3; counter++){
        d_l0[counter]=(1/rho)*z[counter] - (1/m2)*node.k[counter]*(node.o-node.L[node.myHwId-1]) +
            (1/rho/m2)*node.k[counter]*(node.k[node.myHwId-1]*z[node.myHwId-1]-w);
           // Serial.println(d_l0[counter]);
      }
      d_l0[node.myHwId-1] = 0;
      //check feasibility of minimum constrained to linear and 0 boundary
      sol = check_feasibility(d_l0);
       /* Serial.print("FEASIBILITY: ");
        Serial.println(sol);*/
      // compute cost and if best store new optimum
      if (sol){ 
          cost = evaluate_cost(d_l0,rho);
          if (cost < cost_best){
             for(uint8_t counter=0; counter<3; counter++){
              d_best[counter] = d_l0[counter];
             }
             cost_best = cost;
          }
      }
      cs_consensus=CONST_LIN_100;
    }
     break;
     
    // compute minimum constrained to linear and 100 boundary
    case CONST_LIN_100:
    {
      //Serial.println("CONST LIN 100 SOLUTION");
      float d_l1[3] = {};
      for(uint8_t counter=0; counter<3; counter++){
        d_l1[counter]=(1/rho)*z[counter] -(1/m2)*node.k[counter]*(node.o-node.L[node.myHwId-1]+
        100*node.k[node.myHwId-1]) + (1/rho/m2)*node.k[counter]*(node.k[node.myHwId-1]*z[node.myHwId-1]-w);
       // Serial.println(d_l1[counter]);
      }
      d_l1[node.myHwId-1] = 100;
      //check feasibility of minimum constrained to linear and 100 boundary
      sol = check_feasibility(d_l1);
        /*Serial.print("FEASIBILITY: ");
        Serial.println(sol);*/
      // compute cost and if best store new optimum
      if (sol){ 
          cost = evaluate_cost(d_l1, rho);
          if (cost < cost_best){
             for(uint8_t counter=0; counter<3; counter++){
              d_best[counter] = d_l1[counter];
             }
             cost_best = cost;
          }
      }
      cs_consensus= END;
    }
    break;

    case END:
    Serial.println("Best solution: ");
      for(uint8_t counter=0; counter<3; counter++){
        Serial.println(d_best[counter]);
        d[counter]=d_best[counter];
      }
      final_cost=cost_best;
    Serial.println("Best cost: ");
    Serial.println(cost_best);
      cs_consensus=SET;
      partial_solution_ready=1;
    break;
   }
}
void consensus_interface(){
  
  //read
  instruction tempInst;
  if (consensusBuffer.read(tempInst)) {
    switch (tempInst.data[1]) { //purpose byte of can message
      case 9:
        d_all[0][tempInst.data[2]-1]=tempInst.data[0];
      break;
       case 10:
        d_all[1][tempInst.data[2]-1]=tempInst.data[0];
      break;
       case 11:
        d_all[2][tempInst.data[2]-1]=tempInst.data[0];
      break;
      case 12:
        Ready+=1;
      break;
    }
  }
}
 
