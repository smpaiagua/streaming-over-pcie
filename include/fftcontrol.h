

#define FFT_BASE	0x5000
#define FFT_INDEX	(FFT_BASE/0x4)

#define FFT_RST_BASE	0x5100
#define FFT_RST_INDEX	(FFT_RST_BASE/0x4)
#define FFT_RST_CODE	0xA

#define SIZE_MASK	0x1F
#define FWDINV_MASK	0x100
#define SCALING_MASK	0x7FE00

#define FWDINV_SHIFT	8
#define SCALING_SHIFT	9


unsigned int readFFTstatus();

int configFFT(int size,int scaling, int fwdinv);

int setFFTsize(int size);

int setFFTfwdinv(int fwdinv);

int setFFTscaling(int scale_sched);

int FFTreset();
