#ifndef _UNION_HYBRID_H_
#define _UNION_HYBRID_H_

namespace ops{
  template<class CA>
  struct BS_UINT_UNION_VOID{
    inline void init(const CA alpha_in){(void) alpha_in;}
    inline void compute_avx(
      CA * restrict c,
      const size_t c_offset, 
      const CA * restrict a,
      const size_t a_offset, 
      const CA * restrict b,
      const size_t b_offset){
      (void) c; (void) a; (void)b;
      (void) c_offset; (void) a_offset; (void) b_offset;
      return;
    }
    inline void compute_word(
      CA * restrict c,
      const size_t c_offset, 
      const CA * restrict a,
      const size_t a_offset, 
      const CA * restrict b,
      const size_t b_offset){
      (void) c; (void) a; (void)b;
      (void) c_offset; (void) a_offset; (void) b_offset;
      return;
    }
  };

  template<class CA>
  struct BS_UINT_ALPHA_SUM{
    __m256 r_alpha;
    float alpha;
    inline void init(const float alpha_in){}
    inline void compute_avx(
      CA * restrict c,
      const size_t c_offset, 
      const CA * restrict a,
      const size_t a_offset, 
      const CA * restrict b,
      const size_t b_offset) const {
      (void) c; (void) a; (void)b;
      (void) c_offset; (void) a_offset; (void) b_offset;
      return;
    }
    inline void compute_word(
      CA * restrict c,
      const size_t c_offset, 
      const CA * restrict a,
      const size_t a_offset, 
      const CA * restrict b,
      const size_t b_offset) const {
      (void) c; (void) a; (void)b;
      (void) c_offset; (void) a_offset; (void) b_offset;
      return;
    }
  };

  template<>
  void BS_UINT_ALPHA_SUM<float>::init(const float alpha_in) {
    alpha = alpha_in;
    r_alpha = _mm256_set1_ps(alpha_in);
  }

  template<>
  void BS_UINT_ALPHA_SUM<float>::compute_avx(
    float * restrict c,
    const size_t c_offset, 
    const float * restrict a,
    const size_t a_offset, 
    const float * restrict b,
    const size_t b_offset) const {
    const size_t blocks_per_reg = (256/8);
    for(size_t i = 0; i < blocks_per_reg; i++) {
      const __m256 m_b_1 = _mm256_loadu_ps(&a[i*8+a_offset]);
      const __m256 m_b_2 = _mm256_loadu_ps(&b[i*8+b_offset]);
      _mm256_storeu_ps(&c[i*8+c_offset],_mm256_fmadd_ps(m_b_2,r_alpha,m_b_1));
    }
  }

  template<>
  inline void BS_UINT_ALPHA_SUM<float>::compute_word(
    float * restrict c,
    const size_t c_offset, 
    const float * restrict a,
    const size_t a_offset, 
    const float * restrict b,
    const size_t b_offset) const {
    const size_t blocks_per_reg = (64/8);
    for(size_t i = 0; i < blocks_per_reg; i++) {
      const __m256 m_b_1 = _mm256_loadu_ps(&a[i*8+a_offset]);
      const __m256 m_b_2 = _mm256_loadu_ps(&b[i*8+b_offset]);
      _mm256_storeu_ps(&c[i*8+c_offset],_mm256_fmadd_ps(m_b_2,r_alpha,m_b_1));
    }
  }

  /*
  //A must have a range strictly larger than B
  template<class AGG,class CA>
  inline void set_union_hybrid(
      CA value,
      Meta *meta,
      uint64_t * const restrict C, 
      CA * const restrict annoC, 
      const uint64_t * const restrict A,
      const CA * const restrict annoA,
      const uint64_t a_si,
      const uint64_t s_a,
      const uint32_t * const restrict B,
      const CA * const restrict annoB,
      const uint64_t s_b)
{
    (void) s_a;
    size_t count = 0;

    AGG agg;
    agg.init(value);

    const uint32_t bs_start = a_si*64;
    const uint32_t bs_end = (a_si+s_a)*64-1;

    for(size_t i = 0 ; i < s_b; i++){
      const uint32_t data = B[i];
      if(data >= bs_start && data <= bs_end){

      }
    }

    const uint64_t a_start_index = 0;
    const uint64_t b_start_index = a_si-b_si;

    const uint64_t start_index = b_si;
    const uint64_t end_index = (b_si+s_b);
    const uint64_t total_size = (start_index > end_index) ? 0:(end_index-start_index);

    const size_t b_size = total_size*64;
    size_t i = 0;
    while((i+255) < b_size){
      const size_t vector_index = (i/BITS_PER_WORD);
      const __m256 m1 = _mm256_loadu_ps((float*)(A + a_start_index + vector_index));
      const __m256 m2 = _mm256_loadu_ps((float*)(B + b_start_index + vector_index));
      const __m256 r = _mm256_or_ps(m1, m2);
    
      _mm256_storeu_ps((float*)(C+vector_index), r);
      
      count += _mm_popcnt_u64(_mm256_extract_epi64((__m256i)r,0));
      count += _mm_popcnt_u64(_mm256_extract_epi64((__m256i)r,1));
      count += _mm_popcnt_u64(_mm256_extract_epi64((__m256i)r,2));
      count += _mm_popcnt_u64(_mm256_extract_epi64((__m256i)r,3));

      agg.compute_avx(
        annoC,
        vector_index,
        annoA,
        vector_index+a_start_index,
        annoB,
        vector_index+b_start_index);

      i += 256;
    }
    //64 bits per word
    for(; i < b_size; i+=64){
      const size_t vector_index = (i/BITS_PER_WORD);
      const uint64_t r = A[vector_index+a_start_index] | B[vector_index+b_start_index];
      count += _mm_popcnt_u64(r);      
      C[vector_index] = r;

      agg.compute_word(
        annoC,
        vector_index,
        annoA,
        vector_index+a_start_index,
        annoB,
        vector_index+b_start_index);
    }

    meta->cardinality = count;
    meta->type = type::BITSET;
  }
  */
}

#endif