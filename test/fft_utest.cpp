
#include "ntl.h"

template<typename T>
class FFTUtest
{
 public:
  void test_gcd1(GF<T> *gf)
  {
    SignedDoubleT<T> bezout[2];

    assert(gf->inv(20) == 34);
    //
    int i;
    for (i = 0; i < 100; i++) {
      T x = gf->weak_rand();
      assert(1 == gf->arith->extended_gcd(97, x, bezout, NULL));
      // std::cerr << bezout[0] << "*" << 97 << " " << bezout[1] << "*";
      // stdd:cerr << x << "=1\n";
      T y = gf->inv(x);
      // std::cerr << "inv(x)=" << y << "\n";
      if (bezout[1] < 0)
        bezout[1] = bezout[1] + gf->card();
      assert(bezout[1] == y);
    }
  }

  void test_gcd()
  {
    std::cout << "test_gcd\n";

    GFP<T> gf(97);
    test_gcd1(&gf);
  }

  void test_quadratic_residues()
  {
    std::cout << "test_quadratic_residues\n";

    GFP<T> gf32(32);
    int i;
    for (i = 0; i < 32; i++) {
      assert(gf32.is_quadratic_residue(gf32.exp(i, 2)));
    }

    GFP<T> gf7(7);
    assert(gf7.is_quadratic_residue(2));
    assert(!gf7.is_quadratic_residue(5));

    GFP<T> gf8(8);
    assert(gf8.is_quadratic_residue(1));
    assert(!gf8.is_quadratic_residue(3));
  }

  /**
   * convert a number into a vector of digits padded with zeros
   *
   * @param num
   *
   * @return
   */
  Vec<T> *_convert_string2vec(GF<T> *gf, int n, char num[])
  {
    int i;
    Vec<T> *vec = new Vec<T>(gf, n);
    int len = strlen(num);

    for (i = 0; i < len; i++) {
      vec->set(i, num[len - i - 1] - '0');
    }
    for (; i < n; i++) {
      vec->set(i, 0);
    }

    return vec;
  }

  /**
   * Schönhage-Strassen algorithm
   * Example taken from Pierre Meunier's book
   *
   * @param gf
   */
  void test_mul_bignum()
  {
    std::cout << "test_mul_bignum\n";

    int b = 10;  // base
    int p = 14;  // we could multiply integers of 2^p digits
    int max_digits = __arith64.exp(2, p);
    // std::cerr << "p=" << p << " max_digits=" << max_digits << "\n";

    uint64_t l = p + 1;
    // std::cerr << "l=" << l << "\n";

    // choose 2 prime numbers of the form p=a.2^n+1
    // because if x is not a quadratic residue then w=x^a is
    // a 2^n-th principal root of unity in GF_p
    uint64_t a1 = 2;
    uint64_t a2 = 5;
    uint64_t p1 = a1 * __arith64.exp(2, 15) + 1;
    uint64_t p2 = a2 * __arith64.exp(2, 15) + 1;
    // std::cerr << "p1=" << p1 << " p2=" << p2 << "\n";
    assert(__arith64.is_prime(p1));
    assert(__arith64.is_prime(p2));

    // ensure their product is bounded (b-1)^2*2^(n-1) < m
    uint64_t m = p1 * p2;
    // check overflow
    assert(m/p1 == p2);
    // std::cerr << " m=" << m << "\n";
    assert(__arith64.exp((b - 1), 2) * __arith64.exp(p, 2) < m);

    // find x so it is not a quadratic residue in GF_p1 and GF_p2
    assert(__arith64.jacobi(3, p1) == __arith64.jacobi(p1, 3));
    assert(__arith64.jacobi(p1, 3) == __arith64.jacobi(2, 3));
    assert(__arith64.jacobi(3, p2) == __arith64.jacobi(p2, 3));
    assert(__arith64.jacobi(p2, 3) == __arith64.jacobi(2, 3));
    assert(__arith64.jacobi(2, 3) == -1);
    // which means x=3 is not a quadratic residue in GF_p1 and GF_p2

    // therefore we can compute 2^n-th roots of unity in GF_p1 and GF_p2
    uint64_t w1 = __arith64.exp(3, a1);
    uint64_t w2 = __arith64.exp(3, a2);
    // std::cerr << "w1=" << w1 << " w2=" << w2 << "\n";
    assert(w1 == 9);
    assert(w2 == 243);

    // find root of unity in GF_p1p2
    uint64_t _a[2];
    uint64_t _n[2];
    _a[0] = w1;
    _n[0] = p1;
    _a[1] = w2;
    _n[1] = p2;
    uint64_t w = __arith64.chinese_remainder(2, _a, _n);
    // std::cerr << " w=" << w << "\n";
    assert(w == 25559439);

    GFP<T> gf_m(m);
    FFTLN<T> fft(&gf_m, l, 25559439);

    // parse the big numbers
    char X[] = "1236548787985654354598651354984132468";
    char Y[] = "745211515185321545554545854598651354984132468";

    Vec<T> *_X = _convert_string2vec(&gf_m, fft.n, X);
    // _X->dump();
    Vec<T> *_Y = _convert_string2vec(&gf_m, fft.n, Y);
    // _Y->dump();

    Vec<T> *sfX = new Vec<T>(&gf_m, fft.n);
    Vec<T> *sfY = new Vec<T>(&gf_m, fft.n);
    Vec<T> *_XY = new Vec<T>(&gf_m, fft.n);
    Vec<T> *sfXY = new Vec<T>(&gf_m, fft.n);

    fft.fft(sfX, _X);
    fft.fft(sfY, _Y);

    for (int i = 0; i <= fft.n-1; i++) {
      DoubleT<T> val = DoubleT<T>(sfX->get(i)) * sfY->get(i);
      _XY->set(i, val % m);
    }

    fft.ifft(sfXY, _XY);

    // carry propagation
    mpz_class z = 0;
    mpz_class u;
    for (int i = 0; i <= fft.n-1; i++) {
      mpz_class t, b;
      b = 10;
      mpz_pow_ui(t.get_mpz_t(), b.get_mpz_t(), i);
      u = mpz_class(std::to_string(sfXY->get(i)));
      z += u * t;
    }

    // std::cout << z << "\n";
    assert(z.get_str() == std::string("921490395895362412399910100421159322") +
      "712298564831565484737491129935640058571771024");

    delete sfXY;
    delete _XY;
    delete sfX;
    delete sfY;
    delete _X;
    delete _Y;
  }

  void test_fftn()
  {
    u_int n;
    u_int r;
    T q = 65537;
    GFP<T> gf = GFP<T>(q);
    u_int R = gf.get_prime_root();  // primitive root
    u_int n_data = 3;
    u_int n_parities = 3;

    std::cout << "test_fftn\n";

    assert(gf.arith->jacobi(R, q) == -1);

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // compute root of order n-1 such as r^(n-1) mod q == 1
    r = gf.get_nth_root(n);

    // std::cerr << "n=" << n << "\n";
    // std::cerr << "r=" << r << "\n";

    FFTN<T> fft = FFTN<T>(&gf, n, r);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (int j = 0; j < 100000; j++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
      // v.dump();
      fft.fft(&_v, &v);
      // _v.dump();
      fft.ifft(&v2, &_v);
      // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void test_fft2k()
  {
    u_int n;
    u_int q = 65537;
    GFP<T> gf = GFP<T>(q);
    u_int R = gf.get_prime_root();  // primitive root
    u_int n_data = 3;
    u_int n_parities = 3;

    std::cout << "test_fft2k\n";

    assert(gf.arith->jacobi(R, q) == -1);

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // std::cerr << "n=" << n << "\n";

    FFT2K<T> fft = FFT2K<T>(&gf, n);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (int j = 0; j < 100000; j++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
      // v.dump();
      fft.fft(&_v, &v);
      // _v.dump();
      fft.ifft(&v2, &_v);
      // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void test_fftpf()
  {
    T n;
    GF2N<T> gf = GF2N<T>(4);
    T R = gf.get_prime_root();  // primitive root
    T n_data = 3;
    T n_parities = 3;

    std::cout << "test_fftpf\n";

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // std::cerr << "n=" << n << "\n";

    FFTPF<T> fft = FFTPF<T>(&gf, n);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (int j = 0; j < 10000; j++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
        // v.dump();
      fft.fft(&_v, &v);
        // _v.dump();
      fft.ifft(&v2, &_v);
        // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void test_fftct_gfp()
  {
    T n;
    T q = 65537;
    GFP<T> gf = GFP<T>(q);
    T R = gf.get_prime_root();  // primitive root
    T n_data = 3;
    T n_parities = 3;

    std::cout << "test_fftct_gfp\n";

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // std::cerr << "n=" << n << "\n";

    FFTCT<T> fft = FFTCT<T>(&gf, n);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (int j = 0; j < 10000; j++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
        // v.dump();
      fft.fft(&_v, &v);
        // _v.dump();
      fft.ifft(&v2, &_v);
        // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void test_fftct_gf2n()
  {
    T n;
    GF2N<T> gf = GF2N<T>(4);
    T R = gf.get_prime_root();  // primitive root
    T n_data = 3;
    T n_parities = 3;

    std::cout << "test_fftct_gf2n\n";

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // std::cerr << "n=" << n << "\n";

    FFTCT<T> fft = FFTCT<T>(&gf, n);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (int j = 0; j < 10000; j++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
        // v.dump();
      fft.fft(&_v, &v);
        // _v.dump();
      fft.ifft(&v2, &_v);
        // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void test_fft2_gfp()
  {
    T n;
    GFP<T> gf = GFP<T>(3);
    T R = gf.get_prime_root();  // primitive root
    T n_data = 1;
    T n_parities = 1;

    std::cout << "test_fft2_gfp\n";

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // std::cerr << "n=" << n << "\n";

    FFT2<T> fft = FFT2<T>(&gf);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (int j = 0; j < 100000; j++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
        // v.dump();
      fft.fft(&_v, &v);
        // _v.dump();
      fft.ifft(&v2, &_v);
        // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void test_fft2()
  {
    u_int n;
    u_int r;
    u_int q = 65537;
    GFP<T> gf = GFP<T>(q);
    u_int R = gf.get_prime_root();  // primitive root
    u_int n_data = 3;
    u_int n_parities = 3;

    std::cout << "test_fft2\n";

    assert(gf.arith->jacobi(R, q) == -1);

    // with this encoder we cannot exactly satisfy users request, we need to pad
    // n = minimal divisor of (q-1) that is at least (n_parities + n_data)
    n = gf.get_code_len(n_parities + n_data);

    // compute root of order n-1 such as r^(n-1) mod q == 1
    r = gf.get_nth_root(n);

    // std::cerr << "r=" << r << "\n";

    FFTN<T> fft = FFTN<T>(&gf, n, r);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    v.zero_fill();
    for (int i = 0; i < n_data; i++)
      v.set(i, gf.weak_rand());
    v.set(0, 27746);
    v.set(1, 871);
    v.set(2, 49520);
    // v.dump();
    fft.fft(&_v, &v);
    // _v.dump();
    assert(_v.get(0) == 12600);
    assert(_v.get(1) == 27885);
    assert(_v.get(2) == 17398);
    assert(_v.get(3) == 4624);
    assert(_v.get(4) == 10858);
    assert(_v.get(5) == 36186);
    assert(_v.get(6) == 4591);
    assert(_v.get(7) == 42289);
    fft.ifft(&v2, &_v);
    // v2.dump();
    assert(v.eq(&v2));
  }

  void test_fft_gf2n()
  {
    for (int n = 4; n <= 128 && n <= 8 * sizeof(T); n *= 2)
      test_fft_gf2n_with_n(n);
  }

  void test_fft_gf2n_with_n(int n)
  {
    T r;
    GF2N<T> gf = GF2N<T>(n);
    T R = gf.get_prime_root();
    T n_data = 3;
    T n_parities = 3;

    std::cout << "test_fft_gf2n_with_n=" << n << "\n";

    assert(gf.exp(R, gf.card_minus_one()) == 1);

    // with this encoder we cannot exactly satisfy users request, we need to pad
    n = gf.get_code_len(n_data + n_parities);

    r = gf.get_nth_root(n);
    assert(gf.exp(r, n) == 1);

    // std::cerr << "n=" << n << "\n";
    // std::cerr << "r=" << r << "\n";

    FFTN<T> fft = FFTN<T>(&gf, n, r);

    Vec<T> v(&gf, fft.n), _v(&gf, fft.n), v2(&gf, fft.n);
    for (T i = 0; i < 100000; i++) {
      v.zero_fill();
      for (int i = 0; i < n_data; i++)
        v.set(i, gf.weak_rand());
      // v.dump();
      fft.fft(&_v, &v);
      // _v.dump();
      fft.ifft(&v2, &_v);
      // v2.dump();
      assert(v.eq(&v2));
    }
  }

  void fft_utest_no_mul_bignum()
  {
    std::cout << "fft_utest_no_mul_bignum\n";

    test_gcd();
    test_quadratic_residues();
  }

  void fft_utest()
  {
    std::cout << "fft_utest\n";

    test_gcd();
    test_quadratic_residues();
    test_fftn();
    test_fft2k();
    test_fftpf();
    test_fftct_gfp();
    test_fftct_gf2n();
    test_fft2();
    test_fft2_gfp();
    test_fft_gf2n();
    test_mul_bignum();
  }
};

template class Arith<uint32_t>;
template class Mat<uint32_t>;
template class Vec<uint32_t>;
template class FFT<uint32_t>;
template class FFTLN<uint32_t>;
template class FFTPF<uint32_t>;

template class Arith<uint64_t>;
template class Mat<uint64_t>;
template class Vec<uint64_t>;
template class FFT<uint64_t>;
template class FFTPF<uint64_t>;

template class Arith<mpz_class>;
template class Mat<mpz_class>;
template class Vec<mpz_class>;

template class GF<uint32_t>;
template class GFP<uint32_t>;
template class GF2N<uint32_t>;

template class GF<uint64_t>;
template class GFP<uint64_t>;
template class GF2N<uint64_t>;

template class GF<mpz_class>;
template class GFP<mpz_class>;

void fft_utest()
{
  FFTUtest<uint32_t> fftutest_uint32;
  fftutest_uint32.fft_utest_no_mul_bignum();
  FFTUtest<uint64_t> fftutest_uint64;
  fftutest_uint64.fft_utest();
  FFTUtest<mpz_class> fftutest_mpz;
  fftutest_mpz.fft_utest_no_mul_bignum();  // too slow
}
