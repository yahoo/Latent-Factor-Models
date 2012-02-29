/*
 * Copyright (c) 2012, Yahoo! Inc.  All rights reserved.
 * Copyrights licensed under the New BSD License. See the accompanying LICENSE file for terms.
 * Author: Deepak Agarwal
 */


#include "utilR.h"
#include "arms.h"

// main function
double logexp(double x){
  double ans;
  if(x > 0)ans = x + log1p(exp(-x)); else ans = log1p(exp(x));
  return ans;
}

double lgtfn(double b,void *W){
  REG *WW;
  int i;
  double sum,etanew,a,v;

  WW = W;
  sum = 0.0;
  for(i=1;i <= WW->nobs;++i){
    etanew=WW->eta[R_VEC(i)] + WW->X[R_MAT(i,WW->id,WW->nobs)]*(b - WW->betacurr[R_VEC(WW->id)]);
    //printf("etanew=%f\n",etanew);
    sum += ((WW->Y[R_VEC(i)] > 0)? etanew:0) - logexp(etanew);
  }
  // adding log-prior
  a=(b - WW->beta0[R_VEC(WW->id)]);
  v= WW->varbeta[R_VEC(WW->id)];
  sum -= .5*a*a/v;
  return sum;
}

void debuglgt(double *b,double *off,double *beta0,double *varbeta,int *id,
	      double *X,int *Y,int *nFact,int *nObs,double *betaout,double *out){
  REG R;
  double *eta,lin;
  int i,j;

  eta = (double *)Calloc(*nObs,double);
  for(i=1;i <= *nObs;++i){
    eta[R_VEC(i)] = off[R_VEC(i)];
    lin=0.0;
    for(j=1;j <= *nFact;++j){
      lin += X[R_MAT(i,j,*nObs)]*betaout[R_VEC(j)];
    }
    eta[R_VEC(i)] += lin;
  }

  R.eta = eta; R.Y=Y;R.X=X;R.betacurr=betaout;R.beta0=beta0;R.varbeta=varbeta; R.id = *id;R.nobs=*nObs;
  *out = lgtfn(*b,&R);
  Free(eta);
  return;
}
//off: offset
//beta0: priormean
//varbeta: vector of prior variance
//X: vectorized design matrix read columnwise.
//Y: response
// *nFact: number of covariates (including intercept)
// *nObs: number of observations
// betaout: previous beta value
//ninit : number of points in envelope
// xl: lower bound(envelope)
//xu: upper bound(envelope)

// to be fixed: eta is dynamically allocated, this should be done once outside the program and re-used for all calls to this function.
void ARSLOGISTIC(double *off,double *beta0,double *varbeta,
		 double *X,int *Y,int *nFact,int *nObs,double *betaout,
		 int *ninit,double *xl,double *xu,double *eta){
  // betaout can be used as initial values if required.
  // first coordinate of betaout is the intercept.
  //sample betaout
  // for(i=1;i<=*nFact+1;++i)sample betaout[R_VEC(i)]
  double xsamp[1],convex,lin,XL,XU;
  int npoint,nsamp,neval,i,j,flag,count;
  REG R;

  convex=1.0;npoint=500,nsamp=1,neval=0;flag=1;count=0;
  XL = *xl; XU= *xu;
  //eta = (double *)Calloc(*nObs,double);
  for(i=1;i <= *nObs;++i){
    eta[R_VEC(i)] = off[R_VEC(i)];
    lin=0.0;
    for(j=1;j <= *nFact;++j){
      lin += X[R_MAT(i,j,*nObs)]*betaout[R_VEC(j)];
    }
    eta[R_VEC(i)] += lin;
  }
  //printf("xl=%f\n",*xl);
  R.eta = eta; R.Y=Y;R.X=X;R.betacurr=betaout;R.beta0=beta0;R.varbeta=varbeta;R.nobs=*nObs;
  for(j=1;j <= *nFact;++j){
    R.id=j;
    //while(flag!=0 && count <=50){
    //printf("in ars\n");
    flag = arms_simple(*ninit,&XL,&XU,lgtfn,&R,0,&betaout[R_VEC(j)],xsamp);
    if(flag > 0){printf("err=%d\t in ars",flag); exit(1);}
    //flag=arms(xi,*ninit,&XL,&XU,lgtfn,&R,&convex,npoint,0,&betaout[R_VEC(j)],xsamp,nsamp,qcent,xcent,*ncent,&neval);
    // printf("flag=%d\n",flag);
      //count++;
      //}
      //if(count > 50){printf("error in ars\n");exit(1);
      //}
      //flag=1;count=0;
    //update etas
    for(i=1;i <= *nObs;++i)eta[R_VEC(i)] += X[R_MAT(i,j,*nObs)]*(xsamp[0]-betaout[R_VEC(j)]);
    betaout[R_VEC(j)]=xsamp[0];
    R.betacurr[R_VEC(j)] = xsamp[0];
								 
  }
  //Free(eta);
  return;
}



