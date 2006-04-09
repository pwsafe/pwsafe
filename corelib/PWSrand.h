// PWSrand.h
//-----------------------------------------------------------------------------

#ifndef PWSrand_h
#define PWSrand_h
class PWSrand {
 public:
  static PWSrand *GetInstance();
  static void DeleteInstance();

  void AddEntropy(unsigned char *bytes, unsigned int numBytes);
  //  fill this buffer with random data
  void GetRandomData( void * const buffer, unsigned long length );

  //  generate a random number between 0 and len
  unsigned int RangeRand(size_t len);
 private:
  PWSrand(); // start with some minimal entropy
  ~PWSrand();
  static PWSrand *self;
};
#endif // PWSrand_h
