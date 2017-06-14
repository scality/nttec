/* -*- mode: c++ -*- */
#pragma once

/** 
 * GF_2^2^k+1 based RS (Fermat Number Transform)
 * As suggested by the paper:
 * FNT-based Reed-Solomon Erasure Codes
 * by Alexandre Soro and Jerome Lacan
 */
template<typename T>
class FECFNTRS : public FEC<T>
{
private:
  FFT<T> *fft = NULL;

public:
  u_int n;
  u_int N;
  u_int r;

  FECFNTRS(GF<T> *gf, u_int word_size, u_int n_data, u_int n_parities) : 
    FEC<T>(gf, FEC<T>::TYPE_2, word_size, n_data, n_parities)
  {
    u_int q = 65537;
    u_int R = 3; //primitive root
    //order of a number is the lowest power of the number equals 1.
    //if q=65537 in GF_q 3 is a primitive root because is order is q-1: 
    //3^(q-1) = 1 mod q, and for each i such as 0 < i < q-1, 3^i mod q != 1
    assert(gf->_jacobi(R, q) == -1);

    //with this encoder we cannot exactly satisfy users request, we need to pad
    n = __gf64._log2(n_parities + n_data) + 1;
    N = __gf64._exp(2, n);

    //compute root of order N-1 such as r^(N-1) mod q == 1
    //formula given in the paper (very large numbers):
    mpz_class _r = __gfmpz._exp(R, __gfmpz._exp(2, 16-n)) % gf->p;
    r = _r.get_ui();

    //std::cerr << "n=" << n << "\n";
    //std::cerr << "N=" << N << "\n";
    //std::cerr << "r=" << r << "\n";

    this->fft = new FFT<T>(gf, n, r);
  }

  ~FECFNTRS()
  {
    delete fft;
  }

  int get_n_outputs()
  {
    return this->N;
  }

  void encode(Vec<T> *output, std::vector<KeyValue*> props, off_t offset, Vec<T> *words)
  {
    VVec<T> vwords(words, N);
    fft->fft(output, &vwords);
    //check for 65536 value in output
    for (int i = 0;i < N;i++) {
      if (output->get(i) == 65536) {
        char buf[256];
        snprintf(buf, sizeof (buf), "%lu:%d", offset, i);
        assert(nullptr != props[i]);
        props[i]->insert(std::make_pair(buf, "65536"));
      }
    }
    if (offset == 0) {
      std::cout << "encode\n";
      words->dump();
      output->dump();
    }
  }
  
  void decode_add_data(int fragment_index, int row)
  {
    //not applicable
    assert(false);
  }

  void decode_add_parities(int fragment_index, int row)
  {
    //we can't anticipate here
  }

  void decode_build()
  {
    //nothing to do
  }

  /** 
   * Perform a Lagrange interpolation to find the coefficients of the polynomial
   *
   * @note If all fragments are available ifft(words) is enough
   * 
   * @param output must be exactly n_data
   * @param props special values dictionary
   * @param offset used to locate special values
   * @param fragments_ids unused
   * @param words v=(x_0, x_1, ..., x_k-1) k must be exactly n_data
   */
  void decode(Vec<T> *output, std::vector<KeyValue*> props, off_t offset, Vec<T> *fragments_ids, Vec<T> *words)
  {
    //Lagrange interpolation
    Poly<T> A(this->gf), _A(this->gf);

    //compute A(x) = prod_j(x-x_j)
    A.set(0, 1);
    for (int i = 0;i < N;i++) {
      char buf[256];
      snprintf(buf, sizeof (buf), "%lu:%d", offset, i);
      if (nullptr != props[i]) {
        if (props[i]->is_key(buf))
          words->set(i, 65536);
      }
      Poly<T> _t(this->gf), _t2(this->gf);
      _t.set(1, 1);
      T word = words->get(i);
      _t.set(0, word == 0 ? 0 : this->gf->p - words->get(i));
      A.mul(&_t2, &A, &_t);
      A.copy(&_t2);
    }
    //A.dump();

    //compute A'(x) since A_i(x_i) = A'_i(x_i) 
    A.derivative(&_A);

    //evaluate n_i=v_i/A'_i(x_i)
    Vec<T> n(this->gf, N);
    for (int i = 0;i < N;i++) {
      n.set(i, 
            this->gf->div(words->get(i),
                          _A.eval(words->get(i))));
    }
    //n.dump();

    //We have to find the numerator of the following expression:
    //P(x)/A(x) = sum_i=0_k-1(n_i/(x-x_i)) mod x^n    
    //using Taylor series we rewrite the expression into
    //P(x)/A(x) = -sum_i=0_k-1(sum_j=0_n-1(n_i*x_i^(-j-1)*x^j))

    Poly<T> S2(this->gf), _S2(this->gf);
    for (int i = 0;i <= this->n_data-1;i++) {
      Poly<T> S1(this->gf), _S1(this->gf);
      for (int j = 0;j <= N-1;j++) {
        Poly<T> _s(this->gf);
        T tmp1 = this->gf->inv(this->gf->exp(words->get(i), j+1));
        T tmp2 = this->gf->mul(n.get(i), tmp1);
        _s.set(j, tmp2);
        S1.add(&_S1, &S1, &_s);
        S1.copy(&_S1);
      }
      S2.add(&_S2, &S2, &S1);
      S2.copy(&_S2);
    }
    S2.neg(&_S2, &S2);
    if (offset == 0) {
      std::cout << "decode:\n";
      words->dump();
      _S2.dump();
      exit(0);
    }

    //output is n_data length
    for (int i = 0;i < this->n_data;i++)
      output->set(i, _S2.get(i));
  }
};
