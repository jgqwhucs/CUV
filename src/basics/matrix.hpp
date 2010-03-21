/** 
 * @file matrix.hpp
 * @brief general base class for 2D matrices
 * @ingroup basics
 * @author Hannes Schulz
 * @date 2010-03-21
 */
#ifndef __MATRIX_HPP__
#define __MATRIX_HPP__

namespace cuv{
	template<class T>
	struct matrix_traits{
		typedef memory_space memory_space_type;
	};
	template<class V, class I>
	class host_dia_matrix;

	template<class __value_type, class __index_type>
  class matrix
	{
	  public:
		  typedef __value_type value_type;
		  typedef __index_type index_type;
		  template <class Archive, class V, class I> friend void serialize(Archive&, host_dia_matrix<V,I>&, unsigned int) ;
	  protected:
		  index_type m_width;
		  index_type m_height;
		public:
		  matrix(const index_type& h, const index_type& w)
			: m_width(w), m_height(h)
			{
			}
		  inline void resize(const index_type& h, const index_type& w){
			  m_width=w;
			  m_height=h;
		  }
		  inline index_type w()const  { return m_width;                }
		  inline index_type h()const  { return m_height;               }
		  inline index_type n()const  { return w()*h();                }
	};
}

#endif /* __MATRIX_HPP__ */
