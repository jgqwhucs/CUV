//*LB*
// Copyright (c) 2010, University of Bonn, Institute for Computer Science VI
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//  * Neither the name of the University of Bonn 
//    nor the names of its contributors may be used to endorse or promote
//    products derived from this software without specific prior written
//    permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//*LE*





#define BOOST_TEST_MODULE example
#include <boost/test/included/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <limits>

#include <cuv/tools/cuv_general.hpp>
#include <cuv/tensor_ops/tensor_ops.hpp>
#include <cuv/tensor_ops/rprop.hpp>

using namespace cuv;

struct MyConfig {
	static const int dev = CUDA_TEST_DEVICE;
	MyConfig()   { 
		printf("Testing on device=%d\n",dev);
		initCUDA(dev); 
	}
	~MyConfig()  { exitCUDA();  }
};

BOOST_GLOBAL_FIXTURE( MyConfig );

struct Fix{
	tensor<float,dev_memory_space> v,w;
	static const int N = 8092;
	Fix()
	:   v(N),w(N)
	{
	}
	~Fix(){
	}
};


BOOST_FIXTURE_TEST_SUITE( s, Fix )


BOOST_AUTO_TEST_CASE( vec_ops_unary1 )
{
	apply_scalar_functor(v, SF_EXP);
	//apply_scalar_functor(v, SF_EXACT_EXP);
}

BOOST_AUTO_TEST_CASE( vec_ops_binary1 )
{
	sequence(v);
	sequence(w);
	v+= (float) 1.0;
	//apply_scalar_functor(v,SF_ADD,1);
	
	for(int i=0;i<N;i++){
		BOOST_CHECK_EQUAL(v[i], i + 1);
	}
	//apply_binary_functor(v,w, BF_ADD);
	v+=w;
	for(int i=0;i<N;i++){
		BOOST_CHECK_EQUAL(v[i], i + i + 1);
	}
}

BOOST_AUTO_TEST_CASE( vec_ops_scalar_mask )
{
	typedef dev_memory_space ms_type;
	typedef tensor<float,ms_type> tensor_type;
	tensor_type v(N);
	sequence(v);
	tensor<unsigned char,ms_type> mask(v.size());
	apply_scalar_functor(mask,v,SF_LT,10.f); // mark all values in v which are less than 10 as "1"

	// check mask outcome
	for(int i=0;i<10;i++){
		BOOST_CHECK_EQUAL((bool)mask[i], true );
	}
	for(int i=10;i<N;i++){
		BOOST_CHECK_EQUAL((bool)mask[i], false );
	}


	apply_scalar_functor(v,SF_SQUARE, &mask); // square marked values
	// check masked operation outcome
	for(int i=0;i<10;i++){
		BOOST_CHECK_EQUAL(v[i], i*i );
	}
	for(int i=10;i<N;i++){
		BOOST_CHECK_EQUAL(v[i], i );
	}
}

BOOST_AUTO_TEST_CASE( vec_ops_binary2 )
{
	apply_binary_functor(v,w, BF_AXPY, 2.f);
}

BOOST_AUTO_TEST_CASE( vec_ops_scalar_2param )
{
	sequence(v);
	apply_scalar_functor(v, SF_TANH, 3.f, 5.f);
	for(int i=0;i<N;i++){
		BOOST_CHECK_CLOSE((float)v[i], (float)(3*tanh(5*i)), 0.1f);
	}
}

BOOST_AUTO_TEST_CASE( vec_ops_copy )
{
	// generate data
	sequence(w);

	// copy data from v to w
	copy(v,w);
	for(int i=0;i<N;i++){
		BOOST_CHECK_EQUAL(v[i],w[i]);
	}
}

BOOST_AUTO_TEST_CASE( vec_ops_unary_add )
{
	sequence(v);
	apply_scalar_functor(v,SF_ADD,3.8f);
	for(int i=0;i<N;i++){
		BOOST_CHECK_EQUAL(v[i], i+3.8f);
	}
}
BOOST_AUTO_TEST_CASE( vec_ops_unary_add_broadcast1 )
{
	{
        sequence(v);
        w.resize(extents[1]);
        w = 5.f;
        apply_binary_functor(v,w,BF_ADD);
        for(int i=0;i<N;i++){
            BOOST_CHECK_EQUAL(v[i], i+5.f);
        }
	}
	{
        sequence(v);
        w = 5.f;
        apply_binary_functor(v,w,BF_AXPBY, 2.f, 3.f);
        for(int i=0;i<N;i++){
            BOOST_CHECK_EQUAL(v[i], i*2.f + 3.f*5.f);
        }
	}
	{
        sequence(v);
        w = 5.f;
        apply_binary_functor(v,w,BF_XPBY, 2.f);
        for(int i=0;i<N;i++){
            BOOST_CHECK_EQUAL(v[i], i + 2.f*5.f);
        }
	}
}
BOOST_AUTO_TEST_CASE( vec_ops_unary_add_broadcast2 )
{
	{
        sequence(v);
        w.resize(extents[1]);
        w = 5.f;
        apply_binary_functor(v,w,v,BF_ADD);
        for(int i=0;i<N;i++){
            BOOST_CHECK_EQUAL(v[i], i+5.f);
        }
	}
	{
        sequence(v);
        w = 5.f;
        apply_binary_functor(v,w,v,BF_AXPBY, 2.f, 3.f);
        for(int i=0;i<N;i++){
            BOOST_CHECK_EQUAL(v[i], 5.f*2.f + 3.f*i);
        }
	}
	{
        sequence(v);
        w = 5.f;
        apply_binary_functor(v,w,v,BF_XPBY, 2.f);
        for(int i=0;i<N;i++){
            BOOST_CHECK_EQUAL(v[i], 2.f * i + 5.f);
        }
	}
}
BOOST_AUTO_TEST_CASE( vec_ops_axpby )
{
	// generate data
	sequence(w);
	sequence(v);
	apply_scalar_functor(v,SF_ADD, 1.f);
	BOOST_CHECK_CLOSE((float)w[0],0.f,0.1f);
	BOOST_CHECK_CLOSE((float)v[0],1.f,0.1f);

	// copy data from v to w
	apply_binary_functor(v,w,BF_AXPBY, 2.f,3.f);
	for(int i=0;i<N;i++){
		BOOST_CHECK_CLOSE((float)v[i], 2.f*(i+1.f) + 3.f*i, 0.1f );
	}
}


BOOST_AUTO_TEST_CASE( vec_ops_0ary1 )
{
	// test sequence
	sequence(v);
	for(int i=0;i<N;i++){
		BOOST_CHECK_EQUAL(v[i], i);
	}

	// test fill
	fill(w,1.f);
	for(int i=0;i<N;i++){
		BOOST_CHECK_EQUAL(w[i], 1);
	}
}

BOOST_AUTO_TEST_CASE( vec_ops_has_inf )
{
	tensor<float,host_memory_space> v2(N); fill(v2,0.f);
	fill(v,0.f);
	bool no  = has_inf(v);
	bool no2 = has_inf(v2);
	BOOST_CHECK_EQUAL(no,false);
	BOOST_CHECK_EQUAL(no2,false);
	if(std::numeric_limits<float>::has_infinity){
		v[3]=std::numeric_limits<float>::infinity();
		v2[3]=std::numeric_limits<float>::infinity();
		bool yes = has_inf(v);
		bool yes2 = has_inf(v2);
		BOOST_CHECK_EQUAL(yes,true);
		BOOST_CHECK_EQUAL(yes2,true);
	}else{
		BOOST_MESSAGE("Warning: we do not have Inf, skip test!");
	}
}
BOOST_AUTO_TEST_CASE( vec_ops_is_nan )
{
	tensor<float,host_memory_space> v2(N); fill(v2,0.f);
	fill(v,0.f);
	bool no  = has_nan(v);
	bool no2 = has_nan(v2);
	BOOST_CHECK_EQUAL(no,false);
	BOOST_CHECK_EQUAL(no2,false);
	if(std::numeric_limits<float>::has_quiet_NaN){
		v[3]=std::numeric_limits<float>::quiet_NaN();
		v2[3]=std::numeric_limits<float>::quiet_NaN();
		bool yes = has_nan(v);
		bool yes2 = has_nan(v2);
		BOOST_CHECK_EQUAL(yes,true);
		BOOST_CHECK_EQUAL(yes2,true);
	}else{
		BOOST_MESSAGE("Warning: we do not have NaN, skip test!");
	}
}
BOOST_AUTO_TEST_CASE( vec_ops_norms )
{
	sequence(v);
	float f1 = norm1(v), f1_ = 0;
	float f2 = norm2(v), f2_ = 0;
	for(int i=0;i<N;i++){
		f2_ += v[i]*v[i];
		f1_ += fabs(v[i]);
	}
	f2_ = sqrt(f2_);
	BOOST_CHECK_CLOSE(f1,f1_,0.1f);
	BOOST_CHECK_CLOSE(f2,f2_,0.1f);
}

BOOST_AUTO_TEST_CASE( vec_ops_diffnorm )
{
	sequence(v);
	sequence(w);
    w*=2.f;
	float f = diff_norm2(v,w); float g = 0;
	for(int i=0;i<N;i++){
		g += pow(v[i]-w[i],2.f);
	}
	g = sqrt(g);
	BOOST_CHECK_CLOSE(f,g,0.1f);
}

BOOST_AUTO_TEST_CASE( vec_ops_cross_entropy )
{
    for(unsigned int i=0;i<N;i++)
    {
        v[i] = (float)drand48(); // must be a probability
        w[i] = (float)drand48(); // can be anything in |R
    }
    v[0] =  0.f; // special case
    w[0] = -6.f; // special case: large negative value
    v[1] =  0.f; // special case
    w[1] =  6.f; // special case: large positive value
    tensor<float,dev_memory_space> res(v.shape());

	apply_binary_functor(res,v,w,BF_LOGCE_OF_LOGISTIC);
	for(int i=0;i<N;i++){
        double vi = (float)v[i];
        double wi = (float)w[i];
        double  g = -vi*log(1.0/(1.0+exp(-wi))) - (1.0-vi)*log(1.0-1.0/(1.0+exp(-wi)));
        BOOST_CHECK_GT((float)res[i],0.0f);
        BOOST_CHECK_CLOSE((float)res[i],g,0.1f);
	}

	apply_scalar_functor(res,v,SF_LOGADDEXP,0.f);
	for(int i=0;i<N;i++){
        double vi = (float)v[i];
        double  g = log(1.0 + exp(vi));
        BOOST_CHECK_CLOSE((float)res[i],g,0.1f);
	}

	apply_scalar_functor(res,v,SF_LOG1P);
	for(int i=0;i<N;i++){
        double vi = (float)v[i];
        double  g = log(1.0 + vi);
        BOOST_CHECK_CLOSE((float)res[i],g,0.1f);
	}
}

BOOST_AUTO_TEST_CASE( vec_rprop )
{
	tensor<signed char,dev_memory_space> dW_old(N);
	tensor<float,dev_memory_space>       W(N);
	tensor<float,dev_memory_space>       dW(N);
	tensor<float,dev_memory_space>       rate(N);
	fill(W,0.f);
	sequence(dW);           apply_scalar_functor(dW, SF_ADD, -256.f);
	sequence(dW_old);       
	fill(rate, 1.f);
	rprop(W,dW,dW_old,rate);

	tensor<signed char,host_memory_space> h_dW_old(N);
	tensor<float,host_memory_space>       h_W(N);
	tensor<float,host_memory_space>       h_dW(N);
	tensor<float,host_memory_space>       h_rate(N);
	fill(h_W,0.f);
	sequence(h_dW);         apply_scalar_functor(h_dW, SF_ADD, -256.f);
	sequence(h_dW_old);     
	fill(h_rate, 1.f);
	rprop(h_W,h_dW,h_dW_old,h_rate);

	for(int i=0;i<N;i++){
		BOOST_CHECK_CLOSE((float)rate[i],(float)h_rate[i],0.01f);
		BOOST_CHECK_CLOSE((float)W[i],(float)h_W[i],0.01f);
	}
}


BOOST_AUTO_TEST_SUITE_END()
