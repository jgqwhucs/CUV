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
#include <cstdio>
#include <boost/test/included/unit_test.hpp>

#include <cuv_general.hpp>
#include <dense_matrix.hpp>
#include <vector_ops.hpp>
#include <matrix_ops.hpp>
#include <timing.hpp>
#include <random.hpp>
#include <matrix_ops/rprop.hpp>

using namespace cuv;

#define MEASURE_TIME(MSG, OPERATION, ITERS)     \
	float MSG;                                  \
	if(1){                                      \
		Timing tim;                             \
		for(int i=0;i<ITERS;i++){               \
			OPERATION ;                         \
		}                                       \
		tim.update(ITERS);                      \
		printf("%s [%s] took %4.4f us/pass\n", #MSG, #OPERATION, 1000000.0f*tim.perf()); \
		MSG = 1000000.0f*tim.perf();            \
	}

struct MyConfig {
	static const int dev = CUDA_TEST_DEVICE;
	MyConfig()   { 
		printf("Testing on device=%d\n",dev);
		initCUDA(dev); 
		initialize_mersenne_twister_seeds();
	}
	~MyConfig()  { exitCUDA();  }
};


BOOST_GLOBAL_FIXTURE( MyConfig );


struct Fix{
	static const int n = 1024; 
	dense_matrix<float,column_major,dev_memory_space>  u_dev,v_dev,w_dev;
	dense_matrix<float,column_major,host_memory_space> u_host,v_host,w_host;
	Fix()
	:   u_dev(n,n),v_dev(n,n),w_dev(n,n)
	,   u_host(n,n),v_host(n,n),w_host(n,n)
	{
		//MEASURE_TIME("warmup", apply_scalar_functor(v_dev, SF_EXP), 100);
	}
	~Fix(){
	}
};


BOOST_FIXTURE_TEST_SUITE( s, Fix )


BOOST_AUTO_TEST_CASE( mat_prod )
{
	sequence(v_dev); apply_scalar_functor(v_dev,SF_MULT,0.001f);
	sequence(w_dev); apply_scalar_functor(w_dev,SF_MULT,0.001f);
	sequence(v_host); apply_scalar_functor(v_host,SF_MULT,0.001f);
	sequence(w_host); apply_scalar_functor(w_host,SF_MULT,0.001f);
	MEASURE_TIME(dev,  prod(u_dev,v_dev,w_dev, 'n','t'), 10);
	MEASURE_TIME(host, prod(u_host,v_host,w_host, 'n','t'), 10);
	printf("Speedup: %3.4f\n", host/dev);
}

BOOST_AUTO_TEST_CASE( mat_plus_vec )
{
	sequence(v_dev);
	sequence(v_host);
	vector<float,dev_memory_space> v_vec(n); sequence(v_vec);
	vector<float,host_memory_space> x_vec(n); sequence(x_vec);
	MEASURE_TIME(dev,  matrix_plus_col(v_dev,v_vec), 10);
	MEASURE_TIME(host, matrix_plus_col(v_host,x_vec), 10);
	printf("Speedup: %3.4f\n", host/dev);
}

BOOST_AUTO_TEST_CASE( mat_plus_vec_row_maj )
{
	dense_matrix<float,row_major,dev_memory_space> V(v_dev.h(),v_dev.w()); sequence(V);
	dense_matrix<float,row_major,host_memory_space> X(v_host.h(),v_host.w()); sequence(X);
	vector<float,dev_memory_space>   v_vec(n); sequence(v_vec);
	vector<float,host_memory_space>  x_vec(n); sequence(x_vec);
	MEASURE_TIME(dev,  matrix_plus_col(V,v_vec), 10);
	MEASURE_TIME(host, matrix_plus_col(X,x_vec), 10);
	printf("Speedup: %3.4f\n", host/dev);
}

BOOST_AUTO_TEST_CASE( mat_transpose )
{
	const int n = 256;
	const int m = 4096;

	dense_matrix<float,column_major,dev_memory_space> V(n,m), W(m,n); sequence(V);
	dense_matrix<float,column_major,host_memory_space> X(n,m), Y(m,n); sequence(X);

	MEASURE_TIME(dev,  transpose(W,V), 10);
	MEASURE_TIME(host, transpose(Y,X), 10);
	printf("Speedup: %3.4f\n", host/dev);
}

BOOST_AUTO_TEST_CASE( mat_op_reduce_big_rm_to_row )
{
	dense_matrix<float,row_major,dev_memory_space> dA(32, 384*384*32);
	vector<float,dev_memory_space> dV(384*384*32);
	dense_matrix<float,row_major,host_memory_space> hA(32, 384*384*32);
	vector<float,host_memory_space> hV(384*384*32);

//	sequence(dA);
//	sequence(dV);
//	sequence(hA);
//	sequence(hV);

	fill(dA, 1);
	fill(hA, 1);

	fill(dV, 0);
	fill(hV, 0);

	MEASURE_TIME(dev, reduce_to_row(dV,dA,RF_ADD, 1.0f, 1.0f), 10);
	//MEASURE_TIME(host, reduce_to_row(hV,hA,RF_ADD, 1.0f, 1.0f), 10);

}

BOOST_AUTO_TEST_SUITE_END()
