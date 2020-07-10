#ifndef FINAL_AUDIO_H
#define FINAL_AUDIO_H

#include <final_platform_layer.h>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif
#include <math.h> // sin, cos, M_PI
#include <float.h> // EPSILON

#define FFT_CORE_ONLY
#include <fft/fft.hpp>

typedef uint32_t AudioFrameIndex; // The number or index of frames
typedef uint32_t AudioSampleIndex; // The number or index of samples
typedef uint32_t AudioChannelIndex; // The number or index of channels
typedef uint32_t AudioHertz; // The number or index of Hz
typedef size_t AudioSize; // The size in bytes

typedef struct PCMWaveData {
	//! Total frame count
	AudioFrameIndex frameCount;
	//! Samples per second (Frequency in Hz)
	AudioSampleIndex samplesPerSecond;
	//! Bytes per sample
	AudioSize bytesPerSample;
	//! Format type
	fplAudioFormatType formatType;
	//! Number of channels
	AudioChannelIndex channelCount;
	//! Size of samples in bytes
	AudioSize samplesSize;
	//! Samples (Interleaved)
	void *isamples;
	//! Last error string
	char lastError[1024];
	//! Is valid boolean flag
	bool isValid;
} PCMWaveData;

static void FreeWaveData(PCMWaveData *wave) {
	if (wave != fpl_null) {
		if (wave->isamples != fpl_null) {
			fplMemoryFree(wave->isamples);
		}
		fplMemoryClear(wave, sizeof(*wave));
	}
}

static void PushWaveError(PCMWaveData *outWave, const char *format, ...) {
	outWave->lastError[0] = 0;
	va_list argList;
	va_start(argList, format);
	fplFormatStringArgs(outWave->lastError, fplArrayCount(outWave->lastError), format, argList);
	va_end(argList);
}

#define FOURCC32(a, b, c, d) (((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

//
// Forward and Backward FFT
// Ported from: https://github.com/wareya/fft/blob/master/fft.hpp
// Original source cannot handle structs as in/out, so we had to port it (Why wasn`t there a stride).
//
typedef enum FFTDirection {
	FFTDirection_Forward = 0,
	FFTDirection_Backward = 1,
} FFTDirection;

typedef struct FFTDouble {
	double real;
	double imag;
} FFTDouble;

typedef struct FFT {
	FFTDouble *in;
	FFTDouble *out;
	AudioSampleIndex capacity;
	AudioSampleIndex size;
} FFT;

static void FFTCore(const FFTDouble *in, const size_t size, const size_t gap, FFTDouble *out, const FFTDirection direction) {
	if (size == 1) {
		out[0] = in[0];
	} else {
		FFTCore(in, size / 2, gap * 2, out, direction);
		FFTCore(&in[gap], size / 2, gap * 2, &out[size / 2], direction);
		double imagScale = (direction == FFTDirection_Forward) ? -1 : 1;
		for (size_t i = 0; i < size / 2; ++i) {
			// Terms
			double a_real = out[i].real;
			double a_imag = out[i].imag;
			double b_real = out[i + size / 2].real;
			double b_imag = out[i + size / 2].imag;
			double twiddle_real = cos(2 * M_PI * i / size);
			double twiddle_imag = sin(2 * M_PI * i / size) * imagScale;
			// Complex multiplication (vector angle summing and length multiplication)
			double bias_real = b_real * twiddle_real - b_imag * twiddle_imag;
			double bias_imag = b_imag * twiddle_real + b_real * twiddle_imag;
			// Real output (sum of real parts)
			out[i].real = a_real + bias_real;
			out[i + size / 2].real = a_real - bias_real;
			// Imaginary output (sum of imaginary parts)
			out[i].imag = a_imag + bias_imag;
			out[i + size / 2].imag = a_imag - bias_imag;
		}
	}
}

static void NormalizeFFT(FFTDouble *values, const size_t size) {
	if (size > 0) {
		double f = 1.0 / (double)size;
		for (size_t i = 0; i < size; i++) {
			values[i].real *= f;
			values[i].imag *= f;
		}
	}
}

static void HalfNormalizeFFT(FFTDouble *values, const size_t size) {
	if (size > 0) {
		double f = 1.0 / sqrt((double)size);
		for (size_t i = 0; i < size; i++) {
			values[i].real *= f;
			values[i].imag *= f;
		}
	}
}

static void ForwardFFT(const FFTDouble *in, const size_t size, const bool normalized, FFTDouble *out) {
	FFTCore(in, size, 1, out, FFTDirection_Forward);
	if (normalized)
		HalfNormalizeFFT(out, size);
}

static void BackwardFFT(const FFTDouble *in, const size_t size, const bool normalized, FFTDouble *out) {
	FFTCore(in, size, 1, out, FFTDirection_Backward);
	if (normalized)
		HalfNormalizeFFT(out, size);
}

inline bool FFTScalarEquals(const double a, const double b) {
	static const double FFT_EPSILON = 0.00001;
	return fabs(a - b) < FFT_EPSILON;
}

inline bool FFTDoubleEquals(const double expectedReal, const double expectedImag, const double actualReal, const double actualImag) {
	bool result = FFTScalarEquals(expectedReal, actualReal) && FFTScalarEquals(expectedImag, actualImag);
	return(result);
}

static void FFTTest() {
	// This forward FFT without any normalization must return the following values from the input (1,1,1,1,0,0,0,0) with imaginary of zero
	// See https://rosettacode.org/wiki/Fast_Fourier_transform#C.2B.2B for more details.
	// 0 = {real=4.0000000000000000 imag=0.00000000000000000 }
	// 1 = {real=1.0000000000000000 imag=-2.4142135623730949 }
	// 2 = {real=0.00000000000000000 imag=0.00000000000000000 }
	// 3 = {real=1.0000000000000000 imag=-0.41421356237309492 }
	// 4 = {real=0.00000000000000000 imag=0.00000000000000000 }
	// 5 = {real=1.0000000000000000 imag=0.41421356237309515 }
	// 6 = {real=0.00000000000000000 imag=0.00000000000000000 }
	// 7 = {real=0.99999999999999967 imag=2.4142135623730949 }

	double data[8] = { 1,1,1,1,0,0,0,0 };

	FFTDouble dataIn[8];
	for (int i = 0; i < 8; ++i)
		dataIn[i] = fplStructInit(FFTDouble, data[i], 0.0);
	FFTDouble dataOut[8] = { 0 };
	FFTCore(dataIn, 8, 1, dataOut, FFTDirection_Forward);

#if 0
	fplAssert(FFTDoubleEquals(4.0, 0.0, dataOut[0].real, dataOut[0].imag));
	fplAssert(FFTDoubleEquals(1.0, -2.41421, dataOut[1].real, dataOut[1].imag));
	fplAssert(FFTDoubleEquals(0.0, 0.0, dataOut[2].real, dataOut[2].imag));
	fplAssert(FFTDoubleEquals(1.0, -0.414214, dataOut[3].real, dataOut[3].imag));
	fplAssert(FFTDoubleEquals(0.0, 0.0, dataOut[4].real, dataOut[4].imag));
	fplAssert(FFTDoubleEquals(1.0, 0.414214, dataOut[5].real, dataOut[5].imag));
	fplAssert(FFTDoubleEquals(0.0, 0.0, dataOut[6].real, dataOut[6].imag));
	fplAssert(FFTDoubleEquals(1.0, 2.41421, dataOut[7].real, dataOut[7].imag));
#endif
	
}

fpl_inline float AmplitudeToDecibel(const float amplitude) {
	return 20.0f * log10f(amplitude);
}

fpl_inline float DecibelToAmplitude(const float dB) {
	return powf(10.0f, dB / 20.0f);
}

#endif // FINAL_AUDIO_H