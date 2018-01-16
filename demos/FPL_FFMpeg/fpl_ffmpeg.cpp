/*

Custom FFMPEG Media Player Example based on FPL
Written by Torsten Spaete

[x] Reads packets from stream and queues them up
[x] Decodes video and audio packets and queues them up as well
[x] FFMPEG functions are loaded dynamically
[x] Linked list for packet queue
[x] Handle PTS/DTS to schedule video frame
[x] Syncronize video to audio
[x] Fix memory leak (There was no leak at all)
[x] Support for FFMPEG static linking
[x] Rewrite Frame Queue to support Peek in Previous, Current and Next frame
[x] Introduce serials
[x] Introduce null and flush packet
[x] Restart
[x] Frame dropping using prev/next frame
[x] Pause/Resume
[ ] OpenGL Video Rendering
[ ] Syncronize audio to video
[ ] Seeking (+/- 5 secs)
[ ] Composite video rendering
	[ ] OSD
	[ ] Bitmap rect blitting
	[ ] Subtitle Decoding and Compositing
[ ] Image format conversion (YUY2, YUV > RGB24 etc.)
	[ ] Slow CPU implementation
	[ ] SSE2/AVX implementation
	[ ] GLSL
[ ] Audio format conversion (Downsampling, Upsamplíng, S16 > F32 etc.)
	[ ] Slow CPU implementation
	[ ] SSE2/AVX implementation
[ ] UI
	[ ] Current Time
	[ ] Buttons
	[ ] File dialog
	[ ] Seekbar
	[ ] Playlist

Docs:
- http://dranger.com/ffmpeg/tutorial01.html
- https://blogs.gentoo.org/lu_zero/2015/10/15/deprecating-avpicture/
- https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
- https://www.codeproject.com/tips/489450/creating-custom-ffmpeg-io-context

Requirements:
- Custom ffmpeg win64 build from https://ffmpeg.zeranoe.com/builds/

*/

#define FPL_IMPLEMENTATION
#define FPL_AUTO_NAMESPACE
#include "final_platform_layer.hpp"

#include <assert.h> // assert

#include "utils.h"
#include "mpmc_queue.h"

#define PRINT_QUEUE_INFOS 0

#define USE_FFMPEG_STATIC_LINKING 0

//
// FFMPEG headers and function prototypes
//
extern "C" {
#	include <libavcodec/avcodec.h>
#	include <libavformat/avformat.h>
#	include <libavutil/avutil.h>
#	include <libavutil/imgutils.h>
#	include <libswscale\swscale.h>
#	include <libswresample\swresample.h>
#	include <libavutil/time.h>
}

//
// AVFormat
//

// av_register_all
#define FFMPEG_AV_REGISTER_ALL_FUNC(name) void name(void)
typedef FFMPEG_AV_REGISTER_ALL_FUNC(ffmpeg_av_register_all_func);
// avformat_close_input
#define FFMPEG_AVFORMAT_CLOSE_INPUT_FUNC(name) void name(AVFormatContext **s)
typedef FFMPEG_AVFORMAT_CLOSE_INPUT_FUNC(ffmpeg_avformat_close_input_func);
// avformat_open_input
#define FFMPEG_AVFORMAT_OPEN_INPUT_FUNC(name) int name(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options)
typedef FFMPEG_AVFORMAT_OPEN_INPUT_FUNC(ffmpeg_avformat_open_input_func);
// avformat_find_stream_info
#define FFMPEG_AVFORMAT_FIND_STREAM_INFO_FUNC(name) int name(AVFormatContext *ic, AVDictionary **options)
typedef FFMPEG_AVFORMAT_FIND_STREAM_INFO_FUNC(ffmpeg_avformat_find_stream_info_func);
// av_dump_format
#define FFMPEG_AV_DUMP_FORMAT_FUNC(name) void name(AVFormatContext *ic, int index, const char *url, int is_output)
typedef FFMPEG_AV_DUMP_FORMAT_FUNC(ffmpeg_av_dump_format_func);
// av_read_frame
#define FFMPEG_AV_READ_FRAME_FUNC(name) int name(AVFormatContext *s, AVPacket *pkt)
typedef FFMPEG_AV_READ_FRAME_FUNC(ffmpeg_av_read_frame_func);
// avformat_alloc_context
#define FFMPEG_AVFORMAT_ALLOC_CONTEXT_FUNC(name) AVFormatContext *name(void)
typedef FFMPEG_AVFORMAT_ALLOC_CONTEXT_FUNC(ffmpeg_avformat_alloc_context_func);
// avformat_seek_file
#define FFMPEG_AVFORMAT_SEEK_FILE_FUNC(name) int name(AVFormatContext *s, int stream_index, int64_t min_ts, int64_t ts, int64_t max_ts, int flags)
typedef FFMPEG_AVFORMAT_SEEK_FILE_FUNC(ffmpeg_avformat_seek_file_func);
// avformat_match_stream_specifier
#define FFMPEG_AVFORMAT_MATCH_STREAM_SPECIFIER_FUNC(name) int name(AVFormatContext *s, AVStream *st, const char *spec)
typedef FFMPEG_AVFORMAT_MATCH_STREAM_SPECIFIER_FUNC(ffmpeg_avformat_match_stream_specifier_func);
// av_find_best_stream
#define FFMPEG_AV_FIND_BEST_STREAM_FUNC(name) int name(AVFormatContext *ic, enum AVMediaType type, int wanted_stream_nb, int related_stream, AVCodec **decoder_ret, int flags)
typedef FFMPEG_AV_FIND_BEST_STREAM_FUNC(ffmpeg_av_find_best_stream_func);
// av_guess_sample_aspect_ratio
#define FFMPEG_AV_GUESS_SAMPLE_ASPECT_RATIO_FUNC(name) AVRational name(AVFormatContext *format, AVStream *stream, AVFrame *frame)
typedef FFMPEG_AV_GUESS_SAMPLE_ASPECT_RATIO_FUNC(ffmpeg_av_guess_sample_aspect_ratio_func);
// av_guess_frame_rate
#define FFMPEG_AV_GUESS_FRAME_RATE_FUNC(name) AVRational name(AVFormatContext *ctx, AVStream *stream, AVFrame *frame)
typedef FFMPEG_AV_GUESS_FRAME_RATE_FUNC(ffmpeg_av_guess_frame_rate_func);

// av_read_pause
#define FFMPEG_AV_READ_PAUSE_FUNC(name) int name(AVFormatContext *s)
typedef FFMPEG_AV_READ_PAUSE_FUNC(ffmpeg_av_read_pause_func);
// av_read_play
#define FFMPEG_AV_READ_PLAY_FUNC(name) int name(AVFormatContext *s)
typedef FFMPEG_AV_READ_PLAY_FUNC(ffmpeg_av_read_play_func);
// avio_feof
#define FFMPEG_AVIO_FEOF_FUNC(name) int name(AVIOContext *s)
typedef FFMPEG_AVIO_FEOF_FUNC(ffmpeg_avio_feof_func);


//
// AVCodec
//

// avcodec_free_context
#define FFMPEG_AVCODEC_FREE_CONTEXT_FUNC(name) void name(AVCodecContext **avctx)
typedef FFMPEG_AVCODEC_FREE_CONTEXT_FUNC(ffmpeg_avcodec_free_context_func);
// avcodec_alloc_context3
#define FFMPEG_AVCODEC_ALLOC_CONTEXT3_FUNC(name) AVCodecContext *name(const AVCodec *codec)
typedef FFMPEG_AVCODEC_ALLOC_CONTEXT3_FUNC(ffmpeg_avcodec_alloc_context3_func);
// avcodec_parameters_to_context
#define FFMPEG_AVCODEC_PARAMETERS_TO_CONTEXT_FUNC(name) int name(AVCodecContext *codec, const AVCodecParameters *par)
typedef FFMPEG_AVCODEC_PARAMETERS_TO_CONTEXT_FUNC(ffmpeg_avcodec_parameters_to_context_func);
// avcodec_find_decoder
#define FFMPEG_AVCODEC_FIND_DECODER_FUNC(name) AVCodec *name(enum AVCodecID id)
typedef FFMPEG_AVCODEC_FIND_DECODER_FUNC(ffmpeg_avcodec_find_decoder_func);
// avcodec_open2
#define FFMPEG_AVCODEC_OPEN2_FUNC(name) int name(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
typedef FFMPEG_AVCODEC_OPEN2_FUNC(ffmpeg_avcodec_open2_func);
// av_packet_unref
#define FFMPEG_AV_PACKET_UNREF_FUNC(name) void name(AVPacket *pkt)
typedef FFMPEG_AV_PACKET_UNREF_FUNC(ffmpeg_av_packet_unref_func);
// avcodec_receive_frame
#define FFMPEG_AVCODEC_RECEIVE_FRAME_FUNC(name) int name(AVCodecContext *avctx, AVFrame *frame)
typedef FFMPEG_AVCODEC_RECEIVE_FRAME_FUNC(ffmpeg_avcodec_receive_frame_func);
// avcodec_send_packet
#define FFMPEG_AVCODEC_SEND_PACKET_FUNC(name) int name(AVCodecContext *avctx, const AVPacket *avpkt)
typedef FFMPEG_AVCODEC_SEND_PACKET_FUNC(ffmpeg_avcodec_send_packet_func);
// av_packet_alloc
#define FFMPEG_AV_PACKET_ALLOC_FUNC(name) AVPacket *name(void)
typedef FFMPEG_AV_PACKET_ALLOC_FUNC(ffmpeg_av_packet_alloc_func);
// av_packet_free
#define FFMPEG_AV_PACKET_FREE_FUNC(name) void name(AVPacket **pkt)
typedef FFMPEG_AV_PACKET_FREE_FUNC(ffmpeg_av_packet_free_func);
// av_init_packet
#define FFMPEG_AV_INIT_PACKET_FUNC(name) void name(AVPacket *pkt)
typedef FFMPEG_AV_INIT_PACKET_FUNC(ffmpeg_av_init_packet_func);
// avcodec_find_decoder_by_name
#define FFMPEG_AVCODEC_FIND_DECODER_BY_NAME_FUNC(name) AVCodec *name(const char *cname)
typedef FFMPEG_AVCODEC_FIND_DECODER_BY_NAME_FUNC(ffmpeg_avcodec_find_decoder_by_name_func);
// avsubtitle_free
#define FFMPEG_AVSUBTITLE_FREE_FUNC(name) void name(AVSubtitle *sub)
typedef FFMPEG_AVSUBTITLE_FREE_FUNC(ffmpeg_avsubtitle_free_func);
// av_packet_move_ref
#define FFMPEG_AV_PACKET_MOVE_REF_FUNC(name) void name(AVPacket *dst, AVPacket *src)
typedef FFMPEG_AV_PACKET_MOVE_REF_FUNC(ffmpeg_av_packet_move_ref_func);
// av_packet_ref
#define FFMPEG_AV_PACKET_REF_FUNC(name) int name(AVPacket *dst, const AVPacket *src)
typedef FFMPEG_AV_PACKET_REF_FUNC(ffmpeg_av_packet_ref_func);

// avcodec_flush_buffers
#define FFMPEG_AVCODEC_FLUSH_BUFFERS_FUNC(name) void name(AVCodecContext *avctx)
typedef FFMPEG_AVCODEC_FLUSH_BUFFERS_FUNC(ffmpeg_avcodec_flush_buffers_func);
// avcodec_decode_subtitle2
#define FFMPEG_AVCODEC_DECODE_SUBTITLE2_FUNC(name) int name(AVCodecContext *avctx, AVSubtitle *sub, int *got_sub_ptr, AVPacket *avpkt)
typedef FFMPEG_AVCODEC_DECODE_SUBTITLE2_FUNC(ffmpeg_avcodec_decode_subtitle2_func);



//
// AVUtil
//

// av_frame_alloc
#define FFMPEG_AV_FRAME_ALLOC_FUNC(name) AVFrame *name(void)
typedef FFMPEG_AV_FRAME_ALLOC_FUNC(ffmpeg_av_frame_alloc_func);
// av_frame_free
#define FFMPEG_AV_FRAME_FREE_FUNC(name) void name(AVFrame **frame)
typedef FFMPEG_AV_FRAME_FREE_FUNC(ffmpeg_av_frame_free_func);
// av_frame_unref
#define FFMPEG_AV_FRAME_UNREF_FUNC(name) void name(AVFrame *frame)
typedef FFMPEG_AV_FRAME_UNREF_FUNC(ffmpeg_av_frame_unref_func);
// av_frame_move_ref
#define FFMPEG_AV_FRAME_MOVE_REF_FUNC(name) void name(AVFrame *dst, AVFrame *src)
typedef FFMPEG_AV_FRAME_MOVE_REF_FUNC(ffmpeg_av_frame_move_ref_func);
// av_image_get_buffer_size
#define FFMPEG_AV_IMAGE_GET_BUFFER_SIZE_FUNC(name) int name(enum AVPixelFormat pix_fmt, int width, int height, int align)
typedef FFMPEG_AV_IMAGE_GET_BUFFER_SIZE_FUNC(ffmpeg_av_image_get_buffer_size_func);
// av_image_get_linesize
#define FFMPEG_AV_IMAGE_GET_LINESIZE_FUNC(name) int name(enum AVPixelFormat pix_fmt, int width, int plane)
typedef FFMPEG_AV_IMAGE_GET_LINESIZE_FUNC(ffmpeg_av_image_get_linesize_func);
// av_image_fill_arrays
#define FFMPEG_AV_IMAGE_FILL_ARRAYS_FUNC(name) int name(uint8_t *dst_data[4], int dst_linesize[4], const uint8_t *src, enum AVPixelFormat pix_fmt, int width, int height, int align)
typedef FFMPEG_AV_IMAGE_FILL_ARRAYS_FUNC(ffmpeg_av_image_fill_arrays_func);
// av_get_channel_layout_nb_channels
#define FFMPEG_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS_FUNC(name) int name(uint64_t channel_layout)
typedef FFMPEG_AV_GET_CHANNEL_LAYOUT_NB_CHANNELS_FUNC(ffmpeg_av_get_channel_layout_nb_channels_func);
// av_gettime_relative
#define FFMPEG_AV_GETTIME_RELATIVE_FUNC(name) int64_t name(void)
typedef FFMPEG_AV_GETTIME_RELATIVE_FUNC(ffmpeg_av_gettime_relative_func);
// av_gettime
#define FFMPEG_AV_GETTIME_FUNC(name) int64_t name(void)
typedef FFMPEG_AV_GETTIME_FUNC(ffmpeg_av_gettime_func);
// av_get_media_type_string
#define FFMPEG_AV_GET_MEDIA_TYPE_STRING_FUNC(name) const char *name(enum AVMediaType media_type)
typedef FFMPEG_AV_GET_MEDIA_TYPE_STRING_FUNC(ffmpeg_av_get_media_type_string_func);
// av_rescale_q
#define FFMPEG_AV_RESCALE_Q_FUNC(name) int64_t name(int64_t a, AVRational bq, AVRational cq) av_const
typedef FFMPEG_AV_RESCALE_Q_FUNC(ffmpeg_av_rescale_q_func);
// av_samples_get_buffer_size
#define FFMPEG_AV_SAMPLES_GET_BUFFER_SIZE_FUNC(name) int name(int *linesize, int nb_channels, int nb_samples, enum AVSampleFormat sample_fmt, int align)
typedef FFMPEG_AV_SAMPLES_GET_BUFFER_SIZE_FUNC(ffmpeg_av_samples_get_buffer_size_func);
// av_mallocz
#define FFMPEG_AV_MALLOCZ_FUNC(name) void *name(size_t size) av_malloc_attrib av_alloc_size(1)
typedef FFMPEG_AV_MALLOCZ_FUNC(ffmpeg_av_mallocz_func);
// av_freep
#define FFMPEG_AV_FREEP_FUNC(name) void name(void *ptr)
typedef FFMPEG_AV_FREEP_FUNC(ffmpeg_av_freep_func);

//
// SWS
//

// sws_getContext
#define FFMPEG_SWS_GET_CONTEXT_FUNC(name) struct SwsContext *name(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param)
typedef FFMPEG_SWS_GET_CONTEXT_FUNC(ffmpeg_sws_getContext_func);
// sws_getCachedContext
#define FFMPEG_SWS_GET_CACHED_CONTEXT_FUNC(name) struct SwsContext *name(struct SwsContext *context, int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param)
typedef FFMPEG_SWS_GET_CACHED_CONTEXT_FUNC(ffmpeg_sws_getCachedContext_func);

// sws_scale
#define FFMPEG_SWS_SCALE_FUNC(name) int name(struct SwsContext *c, const uint8_t *const srcSlice[], const int srcStride[], int srcSliceY, int srcSliceH, uint8_t *const dst[], const int dstStride[])
typedef FFMPEG_SWS_SCALE_FUNC(ffmpeg_sws_scale_func);
// sws_freeContext
#define FFMPEG_SWS_FREE_CONTEXT_FUNC(name) void name(struct SwsContext *swsContext)
typedef FFMPEG_SWS_FREE_CONTEXT_FUNC(ffmpeg_sws_freeContext_func);

//
// SWR
//

// swr_alloc_set_opts_func
#define FFMPEG_SWR_ALLOC_SET_OPTS(name) struct SwrContext *name(struct SwrContext *s, int64_t out_ch_layout, enum AVSampleFormat out_sample_fmt, int out_sample_rate, int64_t  in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate, int log_offset, void *log_ctx)
typedef FFMPEG_SWR_ALLOC_SET_OPTS(ffmpeg_swr_alloc_set_opts_func);
// swr_free
#define FFMPEG_SWR_FREE(name) void name(struct SwrContext **s)
typedef FFMPEG_SWR_FREE(ffmpeg_swr_free_func);
// swr_convert
#define FFMPEG_SWR_CONVERT(name) int name(struct SwrContext *s, uint8_t **out, int out_count, const uint8_t **in , int in_count)
typedef FFMPEG_SWR_CONVERT(ffmpeg_swr_convert_func);
// swr_init
#define FFMPEG_SWR_INIT(name) int name(struct SwrContext *s)
typedef FFMPEG_SWR_INIT(ffmpeg_swr_init_func);

#define FFMPEG_GET_FUNCTION_ADDRESS(libHandle, libName, target, type, name) \
	target = (type *)GetDynamicLibraryProc(libHandle, name); \
	if (target == nullptr) { \
		ConsoleFormatError("[FFMPEG] Failed getting '%s' from library '%s'!", name, libName); \
		return false; \
	}

struct FFMPEGContext {
	DynamicLibraryHandle avFormatLib;
	DynamicLibraryHandle avCodecLib;
	DynamicLibraryHandle avUtilLib;
	DynamicLibraryHandle swScaleLib;
	DynamicLibraryHandle swResampleLib;

	// Format
	ffmpeg_av_register_all_func *avRegisterAll;
	ffmpeg_avformat_close_input_func *avformatCloseInput;
	ffmpeg_avformat_open_input_func *avformatOpenInput;
	ffmpeg_avformat_find_stream_info_func *avformatFindStreamInfo;
	ffmpeg_av_dump_format_func *avDumpFormat;
	ffmpeg_av_read_frame_func *avReadFrame;
	ffmpeg_avformat_alloc_context_func *avformatAllocContext;
	ffmpeg_avformat_seek_file_func *avformatSeekFile;
	ffmpeg_avformat_match_stream_specifier_func *avformatMatchStreamSpecifier;
	ffmpeg_av_find_best_stream_func *avFindBestStream;
	ffmpeg_av_guess_sample_aspect_ratio_func *avGuessSampleAspectRatio;
	ffmpeg_av_guess_frame_rate_func *avGuessFrameRate;
	ffmpeg_av_read_pause_func *avReadPause;
	ffmpeg_av_read_play_func *avReadPlay;
	ffmpeg_avio_feof_func *avioFEOF;

	// Codec
	ffmpeg_avcodec_free_context_func *avcodecFreeContext;
	ffmpeg_avcodec_alloc_context3_func *avcodecAllocContext3;
	ffmpeg_avcodec_parameters_to_context_func *avcodecParametersToContext;
	ffmpeg_avcodec_find_decoder_func *avcodecFindDecoder;
	ffmpeg_avcodec_open2_func *avcodecOpen2;
	ffmpeg_av_packet_unref_func *avPacketUnref;
	ffmpeg_avcodec_receive_frame_func *avcodecReceiveFrame;
	ffmpeg_avcodec_send_packet_func *avcodecSendPacket;
	ffmpeg_av_packet_alloc_func *avPacketAlloc;
	ffmpeg_av_packet_free_func *avPacketFree;
	ffmpeg_av_init_packet_func *avInitPacket;
	ffmpeg_avsubtitle_free_func *avsubtitleFree;
	ffmpeg_avcodec_find_decoder_by_name_func *avcodecFindDecoderByName;
	ffmpeg_av_packet_move_ref_func *avPacketMoveRef;
	ffmpeg_avcodec_flush_buffers_func *avcodecFlushBuffers;
	ffmpeg_avcodec_decode_subtitle2_func *avcodecDecodeSubtitle2;
	ffmpeg_av_packet_ref_func *avPacketRef;

	// Util
	ffmpeg_av_frame_alloc_func *avFrameAlloc;
	ffmpeg_av_frame_free_func *avFrameFree;
	ffmpeg_av_frame_unref_func *avFrameUnref;
	ffmpeg_av_frame_move_ref_func *avFrameMoveRef;
	ffmpeg_av_image_get_buffer_size_func *avImageGetBufferSize;
	ffmpeg_av_image_get_linesize_func *avImageGetLineSize;
	ffmpeg_av_image_fill_arrays_func *avImageFillArrays;
	ffmpeg_av_get_channel_layout_nb_channels_func *avGetChannelLayoutNBChannels;
	ffmpeg_av_gettime_relative_func *avGetTimeRelative;
	ffmpeg_av_gettime_func *avGetTime;
	ffmpeg_av_get_media_type_string_func *avGetMediaTypeString;
	ffmpeg_av_rescale_q_func *avRescaleQ;
	ffmpeg_av_samples_get_buffer_size_func *avSamplesGetBufferSize;
	ffmpeg_av_mallocz_func *avMallocZ;
	ffmpeg_av_freep_func *avFreeP;

	// SWS
	ffmpeg_sws_getContext_func *swsGetContext;
	ffmpeg_sws_getCachedContext_func *swsGetCachedContext;
	ffmpeg_sws_scale_func *swsScale;
	ffmpeg_sws_freeContext_func *swsFreeContext;

	// SWR
	ffmpeg_swr_alloc_set_opts_func *swrAllocSetOpts;
	ffmpeg_swr_free_func *swrFree;
	ffmpeg_swr_convert_func *swrConvert;
	ffmpeg_swr_init_func *swrInit;
};

static void ReleaseFFMPEG(FFMPEGContext &ffmpeg) {
#if !USE_FFMPEG_STATIC_LINKING
	DynamicLibraryUnload(ffmpeg.swResampleLib);
	DynamicLibraryUnload(ffmpeg.swScaleLib);
	DynamicLibraryUnload(ffmpeg.avUtilLib);
	DynamicLibraryUnload(ffmpeg.avCodecLib);
	DynamicLibraryUnload(ffmpeg.avFormatLib);
#endif
}

static bool LoadFFMPEG(FFMPEGContext &ffmpeg) {
#if !USE_FFMPEG_STATIC_LINKING
	const char *avFormatLibFile = "avformat-58.dll";
	const char *avCodecLibFile = "avcodec-58.dll";
	const char *avUtilLibFile = "avutil-56.dll";
	const char *swScaleLibFile = "swscale-5.dll";
	const char *swResampleLibFile = "swresample-3.dll";

	DynamicLibraryHandle avFormatLib = ffmpeg.avFormatLib = DynamicLibraryLoad(avFormatLibFile);
	DynamicLibraryHandle avCodecLib = ffmpeg.avCodecLib = DynamicLibraryLoad(avCodecLibFile);
	DynamicLibraryHandle avUtilLib = ffmpeg.avUtilLib = DynamicLibraryLoad(avUtilLibFile);
	DynamicLibraryHandle swScaleLib = ffmpeg.swScaleLib = DynamicLibraryLoad(swScaleLibFile);
	DynamicLibraryHandle swResampleLib = ffmpeg.swResampleLib = DynamicLibraryLoad(swResampleLibFile);
#endif

	//
	// AVFormat
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avRegisterAll, ffmpeg_av_register_all_func, "av_register_all");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatCloseInput, ffmpeg_avformat_close_input_func, "avformat_close_input");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatOpenInput, ffmpeg_avformat_open_input_func, "avformat_open_input");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatFindStreamInfo, ffmpeg_avformat_find_stream_info_func, "avformat_find_stream_info");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avDumpFormat, ffmpeg_av_dump_format_func, "av_dump_format");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadFrame, ffmpeg_av_read_frame_func, "av_read_frame");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatAllocContext, ffmpeg_avformat_alloc_context_func, "avformat_alloc_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatSeekFile, ffmpeg_avformat_seek_file_func, "avformat_seek_file");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avformatMatchStreamSpecifier, ffmpeg_avformat_match_stream_specifier_func, "avformat_match_stream_specifier");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avFindBestStream, ffmpeg_av_find_best_stream_func, "av_find_best_stream");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avGuessSampleAspectRatio, ffmpeg_av_guess_sample_aspect_ratio_func, "av_guess_sample_aspect_ratio");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avGuessFrameRate, ffmpeg_av_guess_frame_rate_func, "av_guess_frame_rate");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadPause, ffmpeg_av_read_pause_func, "av_read_pause");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avReadPlay, ffmpeg_av_read_play_func, "av_read_play");
	FFMPEG_GET_FUNCTION_ADDRESS(avFormatLib, avFormatLibFile, ffmpeg.avioFEOF, ffmpeg_avio_feof_func, "avio_feof");
#else
	ffmpeg.avRegisterAll = av_register_all;
	ffmpeg.avformatCloseInput = avformat_close_input;
	ffmpeg.avformatOpenInput = avformat_open_input;
	ffmpeg.avformatFindStreamInfo = avformat_find_stream_info;
	ffmpeg.avDumpFormat = av_dump_format;
	ffmpeg.avReadFrame = av_read_frame;
	ffmpeg.avformatAllocContext = avformat_alloc_context;
	ffmpeg.avformatSeekFile = avformat_seek_file;
	ffmpeg.avformatMatchStreamSpecifier = avformat_match_stream_specifier;
	ffmpeg.avFindBestStream = av_find_best_stream;
	ffmpeg.avGuessSampleAspectRatio = av_guess_sample_aspect_ratio;
	ffmpeg.avGuessFrameRate = av_guess_frame_rate;
	ffmpeg.avReadPause = av_read_pause;
	ffmpeg.avReadPlay = av_read_play;
	ffmpeg.avioFEOF = avio_feof;
#endif

	//
	// AVCodec
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFreeContext, ffmpeg_avcodec_free_context_func, "avcodec_free_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecAllocContext3, ffmpeg_avcodec_alloc_context3_func, "avcodec_alloc_context3");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecParametersToContext, ffmpeg_avcodec_parameters_to_context_func, "avcodec_parameters_to_context");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFindDecoder, ffmpeg_avcodec_find_decoder_func, "avcodec_find_decoder");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecOpen2, ffmpeg_avcodec_open2_func, "avcodec_open2");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketUnref, ffmpeg_av_packet_unref_func, "av_packet_unref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecReceiveFrame, ffmpeg_avcodec_receive_frame_func, "avcodec_receive_frame");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecSendPacket, ffmpeg_avcodec_send_packet_func, "avcodec_send_packet");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketAlloc, ffmpeg_av_packet_alloc_func, "av_packet_alloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketFree, ffmpeg_av_packet_free_func, "av_packet_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avInitPacket, ffmpeg_av_init_packet_func, "av_init_packet");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avsubtitleFree, ffmpeg_avsubtitle_free_func, "avsubtitle_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFindDecoderByName, ffmpeg_avcodec_find_decoder_by_name_func, "avcodec_find_decoder_by_name");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketMoveRef, ffmpeg_av_packet_move_ref_func, "av_packet_move_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecFlushBuffers, ffmpeg_avcodec_flush_buffers_func, "avcodec_flush_buffers");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avcodecDecodeSubtitle2, ffmpeg_avcodec_decode_subtitle2_func, "avcodec_decode_subtitle2");
	FFMPEG_GET_FUNCTION_ADDRESS(avCodecLib, avCodecLibFile, ffmpeg.avPacketRef, ffmpeg_av_packet_ref_func, "av_packet_ref");
#else
	ffmpeg.avcodecFreeContext = avcodec_free_context;
	ffmpeg.avcodecAllocContext3 = avcodec_alloc_context3;
	ffmpeg.avcodecParametersToContext = avcodec_parameters_to_context;
	ffmpeg.avcodecFindDecoder = avcodec_find_decoder;
	ffmpeg.avcodecOpen2 = avcodec_open2;
	ffmpeg.avPacketUnref = av_packet_unref;
	ffmpeg.avcodecReceiveFrame = avcodec_receive_frame;
	ffmpeg.avcodecSendPacket = avcodec_send_packet;
	ffmpeg.avPacketAlloc = av_packet_alloc;
	ffmpeg.avPacketFree = av_packet_free;
	ffmpeg.avInitPacket = av_init_packet;
	ffmpeg.avsubtitleFree = avsubtitle_free;
	ffmpeg.avcodecFindDecoderByName = avcodec_find_decoder_by_name;
	ffmpeg.avPacketMoveRef = av_packet_move_ref;
	ffmpeg.avcodecFlushBuffers = avcodec_flush_buffers;
	ffmpeg.avcodecDecodeSubtitle2 = avcodec_decode_subtitle2;
	ffmpeg.avPacketRef = av_packet_ref;
#endif

	//
	// AVUtil
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameAlloc, ffmpeg_av_frame_alloc_func, "av_frame_alloc");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameFree, ffmpeg_av_frame_free_func, "av_frame_free");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameUnref, ffmpeg_av_frame_unref_func, "av_frame_unref");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFrameMoveRef, ffmpeg_av_frame_move_ref_func, "av_frame_move_ref");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageGetBufferSize, ffmpeg_av_image_get_buffer_size_func, "av_image_get_buffer_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageGetLineSize, ffmpeg_av_image_get_linesize_func, "av_image_get_linesize");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avImageFillArrays, ffmpeg_av_image_fill_arrays_func, "av_image_fill_arrays");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetChannelLayoutNBChannels, ffmpeg_av_get_channel_layout_nb_channels_func, "av_get_channel_layout_nb_channels");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetTimeRelative, ffmpeg_av_gettime_relative_func, "av_gettime_relative");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetTime, ffmpeg_av_gettime_func, "av_gettime");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avGetMediaTypeString, ffmpeg_av_get_media_type_string_func, "av_get_media_type_string");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avRescaleQ, ffmpeg_av_rescale_q_func, "av_rescale_q");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avSamplesGetBufferSize, ffmpeg_av_samples_get_buffer_size_func, "av_samples_get_buffer_size");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avMallocZ, ffmpeg_av_mallocz_func, "av_mallocz");
	FFMPEG_GET_FUNCTION_ADDRESS(avUtilLib, avUtilLibFile, ffmpeg.avFreeP, ffmpeg_av_freep_func, "av_freep");
#else
	ffmpeg.avFrameAlloc = av_frame_alloc;
	ffmpeg.avFrameFree = av_frame_free;
	ffmpeg.avFrameUnref = av_frame_unref;
	ffmpeg.avFrameMoveRef = av_frame_move_ref;
	ffmpeg.avImageGetBufferSize = av_image_get_buffer_size;
	ffmpeg.avImageGetLineSize = av_image_get_linesize;
	ffmpeg.avImageFillArrays = av_image_fill_arrays;
	ffmpeg.avGetChannelLayoutNBChannels = av_get_channel_layout_nb_channels;
	ffmpeg.avGetTimeRelative = av_gettime_relative;
	ffmpeg.avGetTime = av_gettime;
	ffmpeg.avGetMediaTypeString = av_get_media_type_string;
	ffmpeg.avRescaleQ = av_rescale_q;
	ffmpeg.avSamplesGetBufferSize = av_samples_get_buffer_size;
	ffmpeg.avMallocZ = av_mallocz;
	ffmpeg.avFreeP = av_freep;
#endif

	//
	// SWS
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsGetContext, ffmpeg_sws_getContext_func, "sws_getContext");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsScale, ffmpeg_sws_scale_func, "sws_scale");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsFreeContext, ffmpeg_sws_freeContext_func, "sws_freeContext");
	FFMPEG_GET_FUNCTION_ADDRESS(swScaleLib, swScaleLibFile, ffmpeg.swsGetCachedContext, ffmpeg_sws_getCachedContext_func, "sws_getCachedContext");
#else
	ffmpeg.swsGetContext = sws_getContext;
	ffmpeg.swsScale = sws_scale;
	ffmpeg.swsFreeContext = sws_freeContext;
	ffmpeg.swsGetCachedContext = sws_getCachedContext;
#endif

	//
	// SWR
	//
#if !USE_FFMPEG_STATIC_LINKING
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrAllocSetOpts, ffmpeg_swr_alloc_set_opts_func, "swr_alloc_set_opts");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrFree, ffmpeg_swr_free_func, "swr_free");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrConvert, ffmpeg_swr_convert_func, "swr_convert");
	FFMPEG_GET_FUNCTION_ADDRESS(swResampleLib, swResampleLibFile, ffmpeg.swrInit, ffmpeg_swr_init_func, "swr_init");
#else
	ffmpeg.swrAllocSetOpts = swr_alloc_set_opts;
	ffmpeg.swrFree = swr_free;
	ffmpeg.swrConvert = swr_convert;
	ffmpeg.swrInit = swr_init;
#endif

	return true;
}

static FFMPEGContext *globalFFMPEGFunctions = nullptr;

//
// Stats
//
struct MemoryStats {
	volatile int32_t allocatedPackets;
	volatile int32_t usedPackets;
	volatile int32_t allocatedFrames;
	volatile int32_t usedFrames;
};

static MemoryStats globalMemStats = {};

inline void PrintMemStats() {
	int32_t allocatedPackets = AtomicLoadS32(&globalMemStats.allocatedPackets);
	int32_t usedPackets = AtomicLoadS32(&globalMemStats.usedPackets);
	int32_t allocatedFrames = AtomicLoadS32(&globalMemStats.allocatedFrames);
	int32_t usedFrames = AtomicLoadS32(&globalMemStats.usedFrames);
	ConsoleFormatOut("Packets: %d / %d, Frames: %d / %d\n", allocatedPackets, usedPackets, allocatedFrames, usedFrames);
}

//
// Constants
//
// Max number of frames in the queues
fpl_constant uint32_t MAX_VIDEO_FRAME_QUEUE_COUNT = 4;
fpl_constant uint32_t MAX_AUDIO_FRAME_QUEUE_COUNT = 8;
fpl_constant uint32_t MAX_FRAME_QUEUE_COUNT = FPL_MAX(MAX_AUDIO_FRAME_QUEUE_COUNT, MAX_VIDEO_FRAME_QUEUE_COUNT);

// Total size of data from all packet queues
fpl_constant uint64_t MAX_PACKET_QUEUE_SIZE = FPL_MEGABYTES(16);

// Min number of packet frames in a single queue
fpl_constant uint32_t MIN_PACKET_FRAMES = 25;

// External clock min/max frames
fpl_constant uint32_t EXTERNAL_CLOCK_MIN_FRAMES = 2;
fpl_constant uint32_t EXTERNAL_CLOCK_MAX_FRAMES = 10;

// External clock speed adjustment constants for realtime sources based on buffer fullness
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

// No AV sync correction is done if below the minimum AV sync threshold
fpl_constant double AV_SYNC_THRESHOLD_MIN = 0.04;
// No AV sync correction is done if above the maximum AV sync threshold
fpl_constant double AV_SYNC_THRESHOLD_MAX = 0.1;
// No AV correction is done if too big error
fpl_constant double AV_NOSYNC_THRESHOLD = 10.0;
// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
fpl_constant double AV_SYNC_FRAMEDUP_THRESHOLD = 0.1;
// Default refresh rate of 1/sec
fpl_constant double DEFAULT_REFRESH_RATE = 0.01;

//
// Packet Queue
//
static AVPacket globalFlushPacket = {};

struct PacketList {
	AVPacket packet;
	PacketList *next;
	int32_t serial;
};

struct PacketQueue {
	ThreadMutex lock;
	ThreadSignal addedSignal;
	ThreadSignal freeSignal;
	PacketList *first;
	PacketList *last;
	uint64_t size;
	uint64_t duration;
	int32_t packetCount;
	int32_t serial;
};

inline bool IsFlushPacket(PacketList *packet) {
	assert(packet != nullptr);
	bool result = (packet->packet.data == (uint8_t *)&globalFlushPacket);
	return(result);
}

inline PacketList *AllocatePacket(PacketQueue &queue) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	PacketList *packet = (PacketList *)ffmpeg.avMallocZ(sizeof(PacketList));
	if (packet == nullptr) {
		return nullptr;
	}
	AtomicAddS32(&globalMemStats.allocatedPackets, 1);
	return(packet);
}

inline void DestroyPacket(PacketQueue &queue, PacketList *packet) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ffmpeg.avFreeP(packet);
	AtomicAddS32(&globalMemStats.allocatedPackets, -1);
}

inline void ReleasePacketData(PacketList *packet) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	if (!IsFlushPacket(packet)) {
		ffmpeg.avPacketUnref(&packet->packet);
	}
}

inline void ReleasePacket(PacketQueue &queue, PacketList *packet) {
	ReleasePacketData(packet);
	DestroyPacket(queue, packet);
	SignalWakeUp(queue.freeSignal);
}

inline bool AquirePacket(PacketQueue &queue, PacketList *&packet) {
	bool result = false;
	packet = AllocatePacket(queue);
	if (packet != nullptr) {
		result = true;
	}
	return(result);
}

static void FlushPacketQueue(PacketQueue &queue) {
	MutexLock(queue.lock);
	PacketList *p = queue.first;
	while (p != nullptr) {
		PacketList *n = p->next;
		ReleasePacketData(p);
		DestroyPacket(queue, p);
		p = n;
	}
	queue.first = queue.last = nullptr;
	queue.packetCount = 0;
	queue.size = 0;
	queue.duration = 0;
	MutexUnlock(queue.lock);
}

static void DestroyPacketQueue(PacketQueue &queue) {
	FlushPacketQueue(queue);
	SignalDestroy(queue.freeSignal);
	SignalDestroy(queue.addedSignal);
	MutexDestroy(queue.lock);
}

inline bool InitPacketQueue(PacketQueue &queue) {
	queue.lock = MutexCreate();
	if (!queue.lock.isValid) {
		return false;
	}
	queue.addedSignal = SignalCreate();
	if (!queue.addedSignal.isValid) {
		return false;
	}
	queue.freeSignal = SignalCreate();
	if (!queue.freeSignal.isValid) {
		return false;
	}
	return true;
}

inline void PushPacket(PacketQueue &queue, PacketList *packet) {
	MutexLock(queue.lock);
	{
		packet->next = nullptr;
		if (IsFlushPacket(packet)) {
			queue.serial++;
		}
		packet->serial = queue.serial;
		if (queue.first == nullptr) {
			queue.first = packet;
		}
		if (queue.last != nullptr) {
			assert(queue.last->next == nullptr);
			queue.last->next = packet;
		}
		queue.last = packet;
		queue.size += packet->packet.size + sizeof(*packet);
		queue.duration += packet->packet.duration;
		AtomicAddS32(&queue.packetCount, 1);
		AtomicAddS32(&globalMemStats.usedPackets, 1);
		SignalWakeUp(queue.addedSignal);
	}
	MutexUnlock(queue.lock);
}

inline bool PopPacket(PacketQueue &queue, PacketList *&packet) {
	bool result = false;
	MutexLock(queue.lock);
	{
		if (queue.first != nullptr) {
			PacketList *p = queue.first;
			PacketList *n = p->next;
			queue.first = n;
			p->next = nullptr;
			packet = p;
			queue.duration -= packet->packet.duration;
			queue.size -= packet->packet.size + sizeof(*packet);
			if (queue.first == nullptr) {
				queue.last = nullptr;
			}
			AtomicAddS32(&queue.packetCount, -1);
			AtomicAddS32(&globalMemStats.usedPackets, -1);
			result = true;
		}
	}
	MutexUnlock(queue.lock);
	return(result);
}

inline bool PushNullPacket(PacketQueue &queue, int streamIndex) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	bool result = false;
	PacketList *packet = nullptr;
	if (AquirePacket(queue, packet)) {
		ffmpeg.avInitPacket(&packet->packet);
		packet->packet.data = nullptr;
		packet->packet.size = 0;
		packet->packet.stream_index = streamIndex;
		PushPacket(queue, packet);
		result = true;
	}
	return(result);
}

inline bool PushFlushPacket(PacketQueue &queue) {
	bool result = false;
	PacketList *packet = nullptr;
	if (AquirePacket(queue, packet)) {
		packet->packet = globalFlushPacket;
		PushPacket(queue, packet);
		result = true;
	}
	return(result);
}

inline void StartPacketQueue(PacketQueue &queue) {
	MutexLock(queue.lock);
	assert(PushFlushPacket(queue));
	MutexUnlock(queue.lock);
}

//
// Frame Queue
//
struct FrameInfo {
};
struct Frame {
	AVFrame *frame;
	double pts;
	double duration;
	int64_t pos;
	int32_t serial;
	bool isUploaded;
};

// @NOTE(final): This is a single producer single consumer fast ringbuffer queue.
// The read position can never pass the write position and vice versa.
inline AVFrame *AllocateFrame() {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	AVFrame *result = ffmpeg.avFrameAlloc();
	AtomicAddS32(&globalMemStats.allocatedFrames, 1);
	return(result);
}

inline void FreeFrameData(Frame *frame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ffmpeg.avFrameUnref(frame->frame);
}

inline void FreeFrame(Frame *frame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	FreeFrameData(frame);
	ffmpeg.avFrameFree(&frame->frame);
}

struct FrameQueue {
	Frame frames[MAX_FRAME_QUEUE_COUNT];
	ThreadMutex lock;
	ThreadSignal signal;
	PacketList *pendingPacket;
	volatile uint32_t *stopped;
	int32_t readIndex;
	int32_t writeIndex;
	int32_t count;
	int32_t capacity;
	int32_t keepLast;
	int32_t readIndexShown;
	bool isValid;
	bool hasPendingPacket;
};

static bool InitFrameQueue(FrameQueue &queue, int32_t capacity, volatile uint32_t *stopped, int32_t keepLast) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	queue = {};
	queue.capacity = FPL_MIN(capacity, MAX_FRAME_QUEUE_COUNT);
	for (int32_t i = 0; i < queue.capacity; ++i) {
		Frame *frame = queue.frames + i;
		frame->frame = AllocateFrame();
		if (frame->frame == nullptr) {
			return false;
		}
	}

	queue.keepLast = !!keepLast;
	queue.stopped = stopped;

	queue.lock = MutexCreate();
	if (!queue.lock.isValid) {
		return false;
	}

	queue.signal = SignalCreate();
	if (!queue.signal.isValid) {
		return false;
	}

	queue.isValid = true;
	return true;
}

static void DestroyFrameQueue(FrameQueue &queue) {
	SignalDestroy(queue.signal);
	MutexDestroy(queue.lock);
	for (int64_t i = 0; i < queue.capacity; ++i) {
		Frame *frame = queue.frames + i;
		FreeFrame(frame);
	}
}

static Frame *PeekFrameQueue(FrameQueue &queue) {
	return &queue.frames[(queue.readIndex + queue.readIndexShown) % queue.capacity];
}

static Frame *PeekFrameQueueNext(FrameQueue &queue) {
	return &queue.frames[(queue.readIndex + queue.readIndexShown + 1) % queue.capacity];
}

static Frame *PeekFrameQueueLast(FrameQueue &queue) {
	return &queue.frames[queue.readIndex];
}

static bool PeekWritableFromFrameQueue(FrameQueue &queue, Frame *&frame) {
	MutexLock(queue.lock);
	if (queue.count >= queue.capacity || *queue.stopped) {
		MutexUnlock(queue.lock);
		return false;
	}
	MutexUnlock(queue.lock);

	if (*queue.stopped) {
		return false;
	}

	frame = &queue.frames[queue.writeIndex];
	return true;
}

static bool PeekReadableFromFrameQueue(FrameQueue &queue, Frame *&frame) {
	MutexLock(queue.lock);
	if ((queue.count - queue.readIndexShown) <= 0 || *queue.stopped) {
		MutexUnlock(queue.lock);
		return false;
	}
	MutexUnlock(queue.lock);

	if (*queue.stopped) {
		return false;
	}

	frame = &queue.frames[(queue.readIndex + queue.readIndexShown) % queue.capacity];
	return true;
}

static void NextWritable(FrameQueue &queue) {
	queue.writeIndex = (queue.writeIndex + 1) % queue.capacity;

	MutexLock(queue.lock);
	queue.count++;
	SignalWakeUp(queue.signal);
	MutexUnlock(queue.lock);
}

static void NextReadable(FrameQueue &queue) {
	if (queue.keepLast && !queue.readIndexShown) {
		queue.readIndexShown = 1;
		return;
	}

	FreeFrameData(&queue.frames[queue.readIndex]);
	queue.readIndex = (queue.readIndex + 1) % queue.capacity;

	MutexLock(queue.lock);
	queue.count--;
	SignalWakeUp(queue.signal);
	MutexUnlock(queue.lock);
}

static int32_t GetFrameQueueRemainingCount(FrameQueue &queue) {
	return queue.count - queue.readIndexShown;
}

struct MediaStream {
	AVStream *stream;
	AVCodecContext *codecContext;
	AVCodec *codec;
	int32_t streamIndex;
	bool isValid;
};

struct ReaderContext {
	PacketQueue packetQueue;
	ThreadSignal stopSignal;
	ThreadSignal resumeSignal;
	ThreadContext *thread;
	volatile uint32_t readPacketCount;
	volatile uint32_t stopRequest;
	bool isEOF;
};

static bool InitReader(ReaderContext &outReader) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	outReader = {};
	outReader.stopSignal = SignalCreate();
	if (!outReader.stopSignal.isValid) {
		return false;
	}
	outReader.resumeSignal = SignalCreate();
	if (!outReader.resumeSignal.isValid) {
		return false;
	}
	if (!InitPacketQueue(outReader.packetQueue)) {
		return false;
	}
	return true;
}

static void DestroyReader(ReaderContext &reader) {
	DestroyPacketQueue(reader.packetQueue);
	SignalDestroy(reader.resumeSignal);
	SignalDestroy(reader.stopSignal);
}

static void StopReader(ReaderContext &reader) {
	reader.stopRequest = 1;
	SignalWakeUp(reader.stopSignal);
	ThreadWaitForOne(reader.thread);
	ThreadDestroy(reader.thread);
	reader.thread = nullptr;
}

static void StartReader(ReaderContext &reader, run_thread_function *readerThreadFunc, void *state) {
	reader.stopRequest = 0;
	SignalReset(reader.stopSignal);
	assert(reader.thread == nullptr);
	reader.thread = ThreadCreate(readerThreadFunc, state);
}

struct PlayerState;
struct Decoder {
	PacketQueue packetsQueue;
	FrameQueue frameQueue;
	ThreadSignal stopSignal;
	ThreadSignal resumeSignal;
	ThreadContext *thread;
	PlayerState *state;
	ReaderContext *reader;
	MediaStream *stream;
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	volatile uint32_t stopRequest;
	volatile uint32_t isEOF;
	volatile uint32_t decodedFrameCount;
	int32_t pktSerial;
	int32_t finishedSerial;
};

static bool InitDecoder(Decoder &outDecoder, PlayerState *state, ReaderContext *reader, MediaStream *stream, uint32_t frameCapacity, int32_t keepLast) {
	outDecoder = {};
	outDecoder.stream = stream;
	outDecoder.reader = reader;
	outDecoder.state = state;
	outDecoder.stopSignal = SignalCreate();
	outDecoder.pktSerial = -1;
	if (!outDecoder.stopSignal.isValid) {
		return false;
	}
	outDecoder.resumeSignal = SignalCreate();
	if (!outDecoder.resumeSignal.isValid) {
		return false;
	}
	if (!InitPacketQueue(outDecoder.packetsQueue)) {
		return false;
	}
	if (!InitFrameQueue(outDecoder.frameQueue, frameCapacity, &outDecoder.stopRequest, keepLast)) {
		return false;
	}

	outDecoder.start_pts = stream->stream->start_time;
	outDecoder.start_pts_tb = stream->stream->time_base;

	return true;
}

static void DestroyDecoder(Decoder &decoder) {
	DestroyFrameQueue(decoder.frameQueue);
	DestroyPacketQueue(decoder.packetsQueue);
	if (decoder.resumeSignal.isValid) {
		SignalDestroy(decoder.resumeSignal);
	}
	if (decoder.stopSignal.isValid) {
		SignalDestroy(decoder.stopSignal);
	}
}

static ThreadContext *StartDecoder(Decoder &decoder, run_thread_function *decoderThreadFunc) {
	StartPacketQueue(decoder.packetsQueue);
	assert(decoder.thread == nullptr);
	decoder.thread = ThreadCreate(decoderThreadFunc, &decoder);
	return (decoder.thread);
}

static void StopDecoder(Decoder &decoder) {
	decoder.stopRequest = 1;
	if (decoder.stopSignal.isValid) {
		SignalWakeUp(decoder.stopSignal);
	}
	ThreadWaitForOne(decoder.thread);
	ThreadDestroy(decoder.thread);
	decoder.thread = nullptr;
	FlushPacketQueue(decoder.packetsQueue);
}

static void AddPacketToDecoder(Decoder &decoder, PacketList *targetPacket, AVPacket *sourcePacket) {
	targetPacket->packet = *sourcePacket;
	PushPacket(decoder.packetsQueue, targetPacket);
}

//
// Clock
//
struct Clock {
	double pts;
	double ptsDrift;
	double lastUpdated;
	double speed;
	int32_t *queueSerial;
	int32_t serial;
	bool isPaused;
};
namespace AVSyncTypes {
	enum AVSyncTypeEnum {
		AudioMaster,
		VideoMaster,
		ExternalClock,
	};
};
typedef AVSyncTypes::AVSyncTypeEnum AVSyncType;

//
// Video
//
struct Texture {
	uint32_t id;
	uint8_t *data;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t colorBits;
};

static bool InitTexture(Texture &texture, uint32_t w, uint32_t h, uint32_t colorBits, uint8_t *data) {
	texture.id = 1;
	texture.width = w;
	texture.height = h;
	texture.data = data;
	texture.colorBits = colorBits;
	texture.stride = (colorBits / 8) * w;
	return true;
}

static void DestroyTexture(Texture &texture) {
	texture = {};
}

struct VideoContext {
	MediaStream stream;
	Decoder decoder;
	Clock clock;
	Texture targetTexture;
	AVFrame *targetRGBFrame;
	uint8_t *targetRGBBuffer;
	SwsContext *softwareScaleCtx;
};

static void UploadTexture(VideoContext &video, AVFrame *sourceNativeFrame) {
	assert(video.targetTexture.width == sourceNativeFrame->width);
	assert(video.targetTexture.height == sourceNativeFrame->height);
	AVCodecContext *videoCodecCtx = video.stream.codecContext;
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ffmpeg.swsScale(video.softwareScaleCtx, (uint8_t const * const *)sourceNativeFrame->data, sourceNativeFrame->linesize, 0, videoCodecCtx->height, video.targetRGBFrame->data, video.targetRGBFrame->linesize);
	ConvertRGB24ToRGB32(video.targetTexture.data, video.targetTexture.stride, videoCodecCtx->width, videoCodecCtx->height, *video.targetRGBFrame->linesize, video.targetRGBBuffer);
}

//
// Audio
//
struct AudioContext {
	MediaStream stream;
	Decoder decoder;
	Clock clock;
	double audioClock;
	int32_t audioClockSerial;

	SwrContext *softwareResampleCtx;
	Frame *pendingAudioFrame;

	// @NOTE(final): Buffer holding some amount of samples in the format FPL expects, required for doing conversion using swr_convert().
	uint8_t *conversionAudioBuffer;
	uint32_t maxConversionAudioFrameCount;
	uint32_t maxConversionAudioBufferSize;
	uint32_t conversionAudioFramesRemaining;
	uint32_t conversionAudioFrameIndex;
};

struct PlayerPosition {
	bool isValid;
	int64_t value;
};

struct PlayerSettings {
	PlayerPosition startTime;
	PlayerPosition duration;
	int32_t frameDrop;
	bool isInfiniteBuffer;
	bool isLoop;
};

inline void InitPlayerSettings(PlayerSettings &settings) {
	settings.startTime = {};
	settings.duration = {};
	settings.frameDrop = 1;
	settings.isInfiniteBuffer = false;
	settings.isLoop = true;
}

struct SeekState {
	int64_t pos;
	int64_t rel;
	int seekFlags;
	bool isRequired;
};

fpl_constant uint32_t MAX_STREAM_COUNT = 8;
struct PlayerState {
	ReaderContext reader;
	MediaStream stream[MAX_STREAM_COUNT];
	VideoContext video;
	AudioContext audio;
	PlayerSettings settings;
	Clock externalClock;
	SeekState seek;
	AVFormatContext *formatCtx;
	double frameLastPTS;
	double frameLastDelay;
	double frameTimer;
	double maxFrameDuration;
	AVSyncType syncType;
	volatile uint32_t forceRefresh;
	int loop;
	int readPauseReturn;
	int step;
	int frame_drops_early;
	int frame_drops_late;
	bool isInfiniteBuffer;
	bool isRealTime;
	bool isPaused;
	bool lastPaused;
};

inline void PutPacketBackToReader(ReaderContext &reader, PacketList *packet) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ReleasePacket(reader.packetQueue, packet);
}

inline bool StreamHasEnoughPackets(const AVStream *stream, int streamIndex, const PacketQueue &queue) {
	bool result = (streamIndex < 0) ||
		(stream->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
		((queue.packetCount > MIN_PACKET_FRAMES) && (!queue.duration || (av_q2d(stream->time_base) * queue.duration) > 1.0));
	return (result);
}

inline AVSyncType GetMasterSyncType(PlayerState *state) {
	if (state->syncType == AVSyncType::VideoMaster) {
		if (state->video.stream.isValid) {
			return AVSyncType::VideoMaster;
		} else {
			return AVSyncType::AudioMaster;
		}
	} else if (state->syncType == AVSyncType::AudioMaster) {
		if (state->audio.stream.isValid) {
			return AVSyncType::AudioMaster;
		} else {
			return AVSyncType::ExternalClock;
		}
	} else {
		return AVSyncType::ExternalClock;
	}
}

inline double GetClock(Clock &clock) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	if (*clock.queueSerial != clock.serial) {
		return NAN;
	}
	double result;
	if (clock.isPaused) {
		result = clock.pts;
	} else {
		double time = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
		result = clock.ptsDrift + time - (time - clock.lastUpdated) * (1.0 - clock.speed);
	}
	return(result);
}

inline void SetClockAt(Clock &clock, double pts, int32_t serial, double time) {
	clock.pts = pts;
	clock.lastUpdated = time;
	clock.ptsDrift = clock.pts - clock.lastUpdated;
	clock.serial = serial;
}

inline void SetClock(Clock &clock, double pts, int32_t serial) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	double time = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
	SetClockAt(clock, pts, serial, time);
}

inline void SetClockSpeed(Clock &clock, double speed) {
	SetClock(clock, GetClock(clock), clock.serial);
	clock.speed = speed;
}

inline void InitClock(Clock &clock, int32_t *queueSerial) {
	clock.speed = 1.0;
	clock.isPaused = false;
	clock.queueSerial = queueSerial;
	SetClock(clock, NAN, -1);
}

inline void SyncClockToSlave(Clock &c, Clock &slave) {
	double clock = GetClock(c);
	double slaveClock = GetClock(slave);
	if (!isnan(slaveClock) && (isnan(clock) || fabs(clock - slaveClock) > AV_NOSYNC_THRESHOLD)) {
		SetClock(c, slaveClock, slave.serial);
	}
}

inline double GetMasterClock(PlayerState *state) {
	double val;
	switch (GetMasterSyncType(state)) {
		case AVSyncType::VideoMaster:
			val = GetClock(state->video.clock);
			break;
		case AVSyncType::AudioMaster:
			val = GetClock(state->audio.clock);
			break;
		default:
			val = GetClock(state->externalClock);
			break;
	}
	return val;
}

static void UpdateExternalClockSpeed(PlayerState *state) {
	if ((state->video.stream.isValid && state->video.decoder.packetsQueue.packetCount <= EXTERNAL_CLOCK_MIN_FRAMES) ||
		(state->audio.stream.isValid && state->audio.decoder.packetsQueue.packetCount <= EXTERNAL_CLOCK_MIN_FRAMES)) {
		SetClockSpeed(state->externalClock, FPL_MAX(EXTERNAL_CLOCK_SPEED_MIN, state->externalClock.speed - EXTERNAL_CLOCK_SPEED_STEP));
	} else if ((!state->video.stream.isValid || (state->video.decoder.packetsQueue.packetCount > EXTERNAL_CLOCK_MAX_FRAMES)) &&
		(!state->audio.stream.isValid || (state->audio.decoder.packetsQueue.packetCount > EXTERNAL_CLOCK_MAX_FRAMES))) {
		SetClockSpeed(state->externalClock, FPL_MIN(EXTERNAL_CLOCK_SPEED_MAX, state->externalClock.speed + EXTERNAL_CLOCK_SPEED_STEP));
	} else {
		double speed = state->externalClock.speed;
		if (speed != 1.0) {
			SetClockSpeed(state->externalClock, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
		}
	}
}

static void AddFrameToDecoder(Decoder &decoder, Frame *frame, AVFrame *srcFrame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	ffmpeg.avFrameMoveRef(frame->frame, srcFrame);
	NextWritable(decoder.frameQueue);
}

namespace DecodeResults {
	enum DecodeResultEnum {
		Failed = -99,
		Stopped = -1,
		Success = 0,
		RequireMorePackets,
		EndOfStream,
		Skipped,
	};
};
typedef DecodeResults::DecodeResultEnum DecodeResult;

static DecodeResult DecodeFrame(ReaderContext &reader, Decoder &decoder, AVFrame *frame) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	assert(decoder.stream != nullptr);
	AVCodecContext *codecCtx = decoder.stream->codecContext;
	int ret = AVERROR(EAGAIN);
	PacketList *pkt;
	for (;;) {
		if (decoder.packetsQueue.serial == decoder.pktSerial) {
			do {
				if (decoder.isEOF) {
					return DecodeResult::Skipped;
				}
				if (decoder.stopRequest) {
					return DecodeResult::Stopped;
				}

				switch (codecCtx->codec_type) {
					case AVMediaType::AVMEDIA_TYPE_VIDEO:
					{
						ret = ffmpeg.avcodecReceiveFrame(codecCtx, frame);
						if (ret == 0) {
							frame->pts = frame->best_effort_timestamp;
						} else if (ret == AVERROR(EAGAIN)) {
							// This will continue sending packets until the frame is complete
							break;
						}
					} break;

					case AVMediaType::AVMEDIA_TYPE_AUDIO:
					{
						ret = ffmpeg.avcodecReceiveFrame(codecCtx, frame);
						if (ret == 0) {
							AVRational tb = { 1, frame->sample_rate };
							if (frame->pts != AV_NOPTS_VALUE) {
								frame->pts = ffmpeg.avRescaleQ(frame->pts, codecCtx->pkt_timebase, tb);
							} else if (decoder.next_pts != AV_NOPTS_VALUE) {
								frame->pts = ffmpeg.avRescaleQ(decoder.next_pts, decoder.next_pts_tb, tb);
							}
							if (frame->pts != AV_NOPTS_VALUE) {
								decoder.next_pts = frame->pts + frame->nb_samples;
								decoder.next_pts_tb = tb;
							}
						} else if (ret == AVERROR(EAGAIN)) {
							// This will continue sending packets until the frame is complete
							break;
						}
					} break;
				}
				if (ret == 0) {
					return DecodeResult::Success;
				} else if (ret == AVERROR_EOF) {
					decoder.finishedSerial = decoder.pktSerial;
					ffmpeg.avcodecFlushBuffers(codecCtx);
					return DecodeResult::EndOfStream;
				} else if (ret == AVERROR(EAGAIN)) {
					// This will continue sending packets until the frame is complete
					break;
				} else {
					return DecodeResult::Failed;
				}
			} while (ret != AVERROR(EAGAIN));
		}

		do {
			if (decoder.frameQueue.hasPendingPacket) {
				assert(decoder.frameQueue.pendingPacket != nullptr);
				pkt = decoder.frameQueue.pendingPacket;
				decoder.frameQueue.hasPendingPacket = false;
			} else {
				pkt = nullptr;
				if (PopPacket(decoder.packetsQueue, pkt)) {
					decoder.pktSerial = pkt->serial;
				} else {
					// We cannot continue to decode, because the packet queue is empty
					return DecodeResult::RequireMorePackets;
				}
			}
		} while (decoder.packetsQueue.serial != decoder.pktSerial);

		if (pkt != nullptr) {
			if (IsFlushPacket(pkt)) {
				ffmpeg.avcodecFlushBuffers(decoder.stream->codecContext);
				decoder.finishedSerial = 0;
				decoder.next_pts = decoder.start_pts;
				decoder.next_pts_tb = decoder.start_pts_tb;
				PutPacketBackToReader(reader, pkt);
			} else {
				if (ffmpeg.avcodecSendPacket(codecCtx, &pkt->packet) == AVERROR(EAGAIN)) {
					decoder.frameQueue.hasPendingPacket = true;
					decoder.frameQueue.pendingPacket = pkt;
				} else {
					PutPacketBackToReader(reader, pkt);
				}
			}
		}
	}
}

static void QueuePicture(Decoder &decoder, AVFrame *sourceFrame, Frame *targetFrame, int32_t serial) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	assert(targetFrame != nullptr);
	assert(targetFrame->frame != nullptr);
	assert(targetFrame->frame->pkt_size <= 0);
	assert(targetFrame->frame->width == 0);

	AVStream *videoStream = decoder.stream->stream;

	AVRational currentTimeBase = videoStream->time_base;
	AVRational currentFrameRate = ffmpeg.avGuessFrameRate(decoder.state->formatCtx, videoStream, nullptr);

	targetFrame->pos = sourceFrame->pkt_pos;
	targetFrame->pts = (sourceFrame->pts == AV_NOPTS_VALUE) ? NAN : sourceFrame->pts * av_q2d(currentTimeBase);
	targetFrame->duration = (currentFrameRate.num && currentFrameRate.den ? av_q2d({ currentFrameRate.den, currentFrameRate.num }) : 0);
	targetFrame->serial = serial;

	AddFrameToDecoder(decoder, targetFrame, sourceFrame);
}

static void VideoDecodingThreadProc(const ThreadContext &thread, void *userData) {
	Decoder *decoder = (Decoder *)userData;
	assert(decoder != nullptr);

	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ReaderContext &reader = *decoder->reader;

	MediaStream *stream = decoder->stream;
	assert(stream != nullptr);
	assert(stream->isValid);
	assert(stream->streamIndex > -1);

	PlayerState *state = decoder->state;

	ThreadSignal *waitSignals[] = {
		// New packet arrived
		&decoder->packetsQueue.addedSignal,
		// Frame queue changed
		&decoder->frameQueue.signal,
		// Stopped decoding
		&decoder->stopSignal,
		// Resume from sleeping
		&decoder->resumeSignal,
	};

	AVStream *videoStream = decoder->stream->stream;

	AVFrame *sourceFrame = ffmpeg.avFrameAlloc();
	bool hasDecodedFrame = false;
	for (;;) {
		// Wait for any signal (Available packet, Free frame, Stopped, Wake up)
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		// Stop decoder
		if (decoder->stopRequest) {
			break;
		}

		// Wait until the decoder wakes up in the next iteration when the decoder is paused
		if (decoder->isEOF) {
			ThreadSleep(10);
			continue;
		}

		if (!hasDecodedFrame) {
			// Decode video frame
			DecodeResult decodeResult = DecodeFrame(reader, *decoder, sourceFrame);
			if (decodeResult != DecodeResult::Success) {
				if (decodeResult != DecodeResult::RequireMorePackets) {
					ffmpeg.avFrameUnref(sourceFrame);
				}
				if (decodeResult == DecodeResult::EndOfStream) {
					decoder->isEOF = 1;
					continue;
				} else if (decodeResult <= DecodeResult::Stopped) {
					break;
				}

				// Stream finished and no packets left to decode, then are finished as well
				if (reader.isEOF && (decoder->packetsQueue.packetCount == 0)) {
					decoder->isEOF = 1;
				}
			} else {
#if PRINT_QUEUE_INFOS
				uint32_t decodedVideoFrameIndex = AtomicAddU32(&decoder->decodedFrameCount, 1);
				ConsoleFormatOut("Decoded video frame %lu\n", decodedVideoFrameIndex);
#endif
				sourceFrame->sample_aspect_ratio = ffmpeg.avGuessSampleAspectRatio(state->formatCtx, stream->stream, sourceFrame);
				hasDecodedFrame = true;

				if (state->settings.frameDrop > 0 || (state->settings.frameDrop && GetMasterSyncType(state) != AVSyncType::VideoMaster)) {
					double dpts = NAN;
					if (sourceFrame->pts != AV_NOPTS_VALUE) {
						dpts = av_q2d(stream->stream->time_base) * sourceFrame->pts;
					}
					if (!isnan(dpts)) {
						double diff = dpts - GetMasterClock(state);
						if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
							diff < 0 &&
							decoder->pktSerial == state->video.clock.serial &&
							decoder->packetsQueue.packetCount) {
							state->frame_drops_early++;
							ffmpeg.avFrameUnref(sourceFrame);
							hasDecodedFrame = false;
						}
					}
				}
			}
		}

		if (hasDecodedFrame) {
			Frame *targetFrame = nullptr;
			if (PeekWritableFromFrameQueue(decoder->frameQueue, targetFrame)) {
				QueuePicture(*decoder, sourceFrame, targetFrame, decoder->pktSerial);
				ffmpeg.avFrameUnref(sourceFrame);
				hasDecodedFrame = false;
			}
		}

	}
	ffmpeg.avFrameFree(&sourceFrame);
}

static void QueueSamples(Decoder &decoder, AVFrame *sourceFrame, Frame *targetFrame, int32_t serial) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	assert(targetFrame != nullptr);
	assert(targetFrame->frame != nullptr);
	assert(targetFrame->frame->pkt_size <= 0);
	assert(targetFrame->frame->nb_samples == 0);

	AVStream *audioStream = decoder.stream->stream;

	AVRational currentTimeBase = audioStream->time_base;

	targetFrame->pos = sourceFrame->pkt_pos;
	targetFrame->pts = (sourceFrame->pts == AV_NOPTS_VALUE) ? NAN : sourceFrame->pts * av_q2d(currentTimeBase);
	targetFrame->duration = av_q2d({ sourceFrame->nb_samples, sourceFrame->sample_rate });
	targetFrame->serial = serial;

	AddFrameToDecoder(decoder, targetFrame, sourceFrame);
}

static void AudioDecodingThreadProc(const ThreadContext &thread, void *userData) {
	Decoder *decoder = (Decoder *)userData;
	assert(decoder != nullptr);

	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ReaderContext &reader = *decoder->reader;

	PlayerState *state = decoder->state;
	assert(state != nullptr);

	MediaStream *stream = decoder->stream;
	assert(stream != nullptr);
	assert(stream->isValid);
	assert(stream->streamIndex > -1);

	ThreadSignal *waitSignals[] = {
		// New packet arrived
		&decoder->packetsQueue.addedSignal,
		// Frame queue changed
		&decoder->frameQueue.signal,
		// Stopped decoding
		&decoder->stopSignal,
		// Resume from sleeping
		&decoder->resumeSignal,
	};

	AVFrame *sourceFrame = ffmpeg.avFrameAlloc();
	bool hasDecodedFrame = false;
	for (;;) {
		// Wait for any signal (Available packet, Free frame, Stopped, Wake up)
		SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));

		// Stop decoder
		if (decoder->stopRequest) {
			break;
		}

		// Wait until the decoder wakes up in the next iteration when the decoder is paused
		if (decoder->isEOF) {
			continue;
		}

		if (!hasDecodedFrame) {
			// Decode video frame
			DecodeResult decodeResult = DecodeFrame(reader, *decoder, sourceFrame);
			if (decodeResult != DecodeResult::Success) {
				if (decodeResult != DecodeResult::RequireMorePackets) {
					ffmpeg.avFrameUnref(sourceFrame);
				}
				if (decodeResult == DecodeResult::EndOfStream) {
					decoder->isEOF = 1;
					continue;
				} else if (decodeResult <= DecodeResult::Stopped) {
					break;
				}

				// Stream finished and no packets left to decode, then are finished as well
				if (reader.isEOF && (decoder->packetsQueue.packetCount == 0)) {
					decoder->isEOF = 1;
				}
			} else {
#if PRINT_QUEUE_INFOS
				uint32_t decodedAudioFrameIndex = AtomicAddU32(&decoder->decodedFrameCount, 1);
				ConsoleFormatOut("Decoded audio frame %lu\n", decodedAudioFrameIndex);
#endif
				hasDecodedFrame = true;
			}
		}

		if (hasDecodedFrame) {
			Frame *targetFrame = nullptr;
			if (PeekWritableFromFrameQueue(decoder->frameQueue, targetFrame)) {
				QueueSamples(*decoder, sourceFrame, targetFrame, decoder->pktSerial);

				// Update decoder audio clock
				if (!isnan(targetFrame->pts)) {
					state->audio.audioClock = targetFrame->pts + (double)targetFrame->frame->nb_samples / (double)targetFrame->frame->sample_rate;
				} else {
					state->audio.audioClock = NAN;
				}
				state->audio.audioClockSerial = targetFrame->serial;

				ffmpeg.avFrameUnref(sourceFrame);
				hasDecodedFrame = false;
			}
		}
	}
	ffmpeg.avFrameFree(&sourceFrame);
}

static void WriteSilenceSamples(AudioContext *audio, uint32_t remainingFrameCount, uint32_t outputSampleStride, uint8_t *conversionAudioBuffer) {
	audio->conversionAudioFramesRemaining = remainingFrameCount;
	audio->conversionAudioFrameIndex = 0;
	size_t bytesToClear = remainingFrameCount * outputSampleStride;
	MemoryClear(conversionAudioBuffer, bytesToClear);

}

static uint32_t AudioReadCallback(const AudioDeviceFormat &nativeFormat, const uint32_t frameCount, void *outputSamples, void *userData) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	double audioCallbackTime = (double)ffmpeg.avGetTimeRelative();

	// Intermedite PCM
	// Sample0[Left], Sample0[Right], Sample1[Left], Sample1[Right],...
	// Frame0[Sample Left][Sample Right], Frame1[Sample Left][Sample Right], Frame2[Sample Left][Sample Right],...
	// Samples per Channel = Number of frames
	AudioContext *audio = (AudioContext *)userData;
	assert(audio != nullptr);

	Decoder &decoder = audio->decoder;

	PlayerState *state = decoder.state;

	uint32_t result = 0;

	if (audio->stream.isValid) {
		uint8_t *conversionAudioBuffer = audio->conversionAudioBuffer;
		uint32_t maxConversionAudioBufferSize = audio->maxConversionAudioBufferSize;

		uint32_t outputSampleStride = nativeFormat.channels * audio::GetAudioSampleSizeInBytes(nativeFormat.type);
		uint32_t maxOutputSampleBufferSize = outputSampleStride * frameCount;

		uint32_t remainingFrameCount = frameCount;
		while (remainingFrameCount > 0) {
			if (state->isPaused) {
				WriteSilenceSamples(audio, remainingFrameCount, outputSampleStride, conversionAudioBuffer);
			}

			// Consume audio in conversion buffer before we do anything else
			if ((audio->conversionAudioFramesRemaining) > 0) {
				uint32_t maxFramesToRead = audio->conversionAudioFramesRemaining;
				uint32_t framesToRead = FPL_MIN(remainingFrameCount, maxFramesToRead);
				size_t bytesToCopy = framesToRead * outputSampleStride;

				assert(audio->conversionAudioFrameIndex < audio->maxConversionAudioFrameCount);
				size_t sourcePosition = audio->conversionAudioFrameIndex * outputSampleStride;
				assert(sourcePosition < audio->maxConversionAudioBufferSize);

				size_t destPosition = (frameCount - remainingFrameCount) * outputSampleStride;
				assert(destPosition < maxOutputSampleBufferSize);

				memory::MemoryCopy(conversionAudioBuffer + sourcePosition, bytesToCopy, (uint8_t *)outputSamples + destPosition);

				remainingFrameCount -= framesToRead;
				audio->conversionAudioFrameIndex += framesToRead;
				audio->conversionAudioFramesRemaining -= framesToRead;
				result += framesToRead;
			}

			// If we consumed all remaining audio frames, then we are done.
			if (remainingFrameCount == 0) {
				// @NOTE(final): Its highly possible that there are frames left in the conversion buffer, so dont clear anything here!
				break;
			}

			// Convert entire pending frame into conversion buffer
			if (audio->pendingAudioFrame != nullptr) {
				assert(audio->conversionAudioFramesRemaining == 0);
				Frame *audioFrame = audio->pendingAudioFrame;
				assert(audioFrame->frame != nullptr);

				audio->pendingAudioFrame = nullptr;

				uint32_t sourceSampleCount = audioFrame->frame->nb_samples;
				uint32_t sourceChannels = audioFrame->frame->channels;
				uint32_t sourceFrameCount = sourceSampleCount;
				uint8_t **sourceSamples = audioFrame->frame->extended_data;

				// Conversion buffer needs to be big enough to hold the samples for the frame
				uint32_t maxConversionSampleCount = audio->maxConversionAudioFrameCount;
				assert(sourceSampleCount <= maxConversionSampleCount);

				int samplesPerChannel = ffmpeg.swrConvert(audio->softwareResampleCtx, (uint8_t **)&audio->conversionAudioBuffer, maxConversionSampleCount, (const uint8_t **)sourceSamples, sourceSampleCount);

				NextReadable(decoder.frameQueue);

				if (samplesPerChannel <= 0) {
					break;
				}

				audio->conversionAudioFramesRemaining = samplesPerChannel;
				audio->conversionAudioFrameIndex = 0;
			}

			if ((audio->pendingAudioFrame == nullptr) && (audio->conversionAudioFramesRemaining == 0)) {
				Frame *newAudioFrame;
				if (!state->isPaused && PeekReadableFromFrameQueue(decoder.frameQueue, newAudioFrame)) {
					if (newAudioFrame->serial != decoder.packetsQueue.serial) {
						NextReadable(decoder.frameQueue);
						continue;
					}
					audio->pendingAudioFrame = newAudioFrame;
					audio->conversionAudioFrameIndex = 0;
					audio->conversionAudioFramesRemaining = 0;
					continue;
				} else {
					// No audio frame available, write silence for the remaining frames
					if (remainingFrameCount > 0) {
						WriteSilenceSamples(audio, remainingFrameCount, outputSampleStride, conversionAudioBuffer);
					} else {
						break;
					}
				}
			}
		}

		// Update audio clock
		if (!isnan(audio->audioClock)) {
			uint32_t writtenSize = result * outputSampleStride;
			uint32_t bytesPerSample = nativeFormat.sampleRate * outputSampleStride;
			double diff = (double)(nativeFormat.periods * nativeFormat.bufferSizeInBytes + writtenSize) / (double)bytesPerSample;
			SetClockAt(audio->clock, audio->audioClock - diff, audio->audioClockSerial, audioCallbackTime / (double)AV_TIME_BASE);
			SyncClockToSlave(state->externalClock, audio->clock);
		}

	}

	return(result);
}

static void StreamTogglePause(PlayerState *state) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	if (state->isPaused) {
		state->frameTimer += ffmpeg.avGetTimeRelative() / 1000000.0 - state->video.clock.lastUpdated;
		if (state->readPauseReturn != AVERROR(ENOSYS)) {
			state->video.clock.isPaused = false;
		}
		SetClock(state->video.clock, GetClock(state->video.clock), state->video.clock.serial);
	}
	SetClock(state->externalClock, GetClock(state->externalClock), state->externalClock.serial);
	state->isPaused = state->audio.clock.isPaused = state->video.clock.isPaused = state->externalClock.isPaused = !state->isPaused;
}

static void SeekStream(SeekState *state, int64_t pos, int64_t rel, bool seekInBytes) {
	if (!state->isRequired) {
		state->pos = pos;
		state->rel = rel;
		state->seekFlags &= ~AVSEEK_FLAG_BYTE;
		if (seekInBytes)
			state->seekFlags |= AVSEEK_FLAG_BYTE;
		state->isRequired = 1;
	}
}

static void TogglePause(PlayerState *state) {
	StreamTogglePause(state);
	state->step = false;
}

static void StepToNextFrame(PlayerState *state) {
	if (state->isPaused) {
		StreamTogglePause(state);
	}
	state->step = 1;
}

static void PacketReadThreadProc(const ThreadContext &thread, void *userData) {
	PlayerState *state = (PlayerState *)userData;
	assert(state != nullptr);

	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	ReaderContext &reader = state->reader;
	VideoContext &video = state->video;
	AudioContext &audio = state->audio;
	MediaStream *videoStream = video.decoder.stream;
	MediaStream *audioStream = audio.decoder.stream;
	AVFormatContext *formatCtx = state->formatCtx;
	assert(formatCtx != nullptr);

	ThreadSignal *waitSignals[] = {
		// We got a free packet for use to read into
		&reader.packetQueue.freeSignal,
		// Reader should terminate
		&reader.stopSignal,
		// Reader can continue
		&reader.resumeSignal,
	};

	bool skipWait = true;
	AVPacket srcPacket;
	bool hasPendingPacket = false;
	for (;;) {
		// Wait for any signal or skip wait
		if (!skipWait) {
			SignalWaitForAny(waitSignals, FPL_ARRAYCOUNT(waitSignals));
		} else {
			skipWait = false;
		}

		// Stop reader
		if (reader.stopRequest) {
			break;
		}

		// Pause
		if (state->isPaused != state->lastPaused) {
			state->lastPaused = state->isPaused;
			if (state->isPaused) {
				state->readPauseReturn = ffmpeg.avReadPause(formatCtx);
			} else {
				ffmpeg.avReadPlay(formatCtx);
			}
		}

		// Seeking
		if (state->seek.isRequired) {
			int64_t seekTarget = state->seek.pos;
			int64_t seekMin = state->seek.rel > 0 ? seekTarget - state->seek.rel + 2 : INT64_MIN;
			int64_t seekMax = state->seek.rel < 0 ? seekTarget - state->seek.rel - 2 : INT64_MAX;
			int seekResult = ffmpeg.avformatSeekFile(formatCtx, -1, seekMin, seekTarget, seekMax, state->seek.seekFlags);
			if (seekResult < 0) {
				// @TODO(final): Log seek error
			} else {
				if (state->seek.seekFlags & AVSEEK_FLAG_BYTE) {
					SetClock(state->externalClock, NAN, 0);
				} else {
					SetClock(state->externalClock, seekTarget / (double)AV_TIME_BASE, 0);
				}
				if (state->audio.stream.isValid) {
					FlushPacketQueue(state->audio.decoder.packetsQueue);
					PushFlushPacket(state->audio.decoder.packetsQueue);

					state->audio.decoder.isEOF = false;
					SignalWakeUp(state->audio.decoder.resumeSignal);
				}
				if (state->video.stream.isValid) {
					FlushPacketQueue(state->video.decoder.packetsQueue);
					PushFlushPacket(state->video.decoder.packetsQueue);

					state->video.decoder.isEOF = false;
					SignalWakeUp(state->video.decoder.resumeSignal);
				}
			}
			state->seek.isRequired = false;
			reader.isEOF = false;
			if (state->isPaused) {
				StepToNextFrame(state);
			}
		}

		// @TODO(final): Handle attached pictures

		// Limit the queue?
		if ((!state->isInfiniteBuffer &&
			(audio.decoder.packetsQueue.size + video.decoder.packetsQueue.size) > MAX_PACKET_QUEUE_SIZE) ||
			 (StreamHasEnoughPackets(audio.stream.stream, audio.stream.streamIndex, audio.decoder.packetsQueue) &&
			  StreamHasEnoughPackets(video.stream.stream, video.stream.streamIndex, video.decoder.packetsQueue))) {
			skipWait = true;
			ThreadSleep(10);
			continue;
		}

		//
		// Seek to the beginning when everything is done
		//
		// @TODO(final): Make this configurable
		bool autoExit = true;
		int64_t startTime = AV_NOPTS_VALUE;

		if (!state->isPaused &&
			(!state->audio.stream.isValid || (state->audio.decoder.finishedSerial == state->audio.decoder.packetsQueue.serial && GetFrameQueueRemainingCount(state->audio.decoder.frameQueue) == 0)) &&
			(!state->video.stream.isValid || (state->video.decoder.finishedSerial == state->video.decoder.packetsQueue.serial && GetFrameQueueRemainingCount(state->video.decoder.frameQueue) == 0))) {
			if ((state->loop == -1) || (state->loop > 0)) {
				if (state->loop > 0) {
					--state->loop;
				}
				SeekStream(&state->seek, startTime != AV_NOPTS_VALUE ? startTime : 0, 0, 0);
			} else {
				if (autoExit) {
					break;
				}
			}
		}

		// Read packet
		if (!hasPendingPacket) {
			int res = ffmpeg.avReadFrame(formatCtx, &srcPacket);
			if (res < 0) {
				if ((res == AVERROR_EOF || ffmpeg.avioFEOF(formatCtx->pb)) && !reader.isEOF) {
					if (video.stream.isValid) {
						PushNullPacket(video.decoder.packetsQueue, video.stream.streamIndex);
					}
					if (audio.stream.isValid) {
						PushNullPacket(audio.decoder.packetsQueue, audio.stream.streamIndex);
					}
					reader.isEOF = true;
				}
				if (formatCtx->pb != nullptr && formatCtx->pb->error) {
					// @TODO(final): Handle error
					break;
				}

				// Wait for a few milliseconds
				ThreadSleep(10);
				skipWait = true;
				continue;
			} else {
				hasPendingPacket = true;
				reader.isEOF = false;
			}
		}

		if (hasPendingPacket) {
			// Try to get new packet from the freelist
			PacketList *targetPacket = nullptr;
			if (AquirePacket(reader.packetQueue, targetPacket)) {
				assert(targetPacket != nullptr);

#if PRINT_QUEUE_INFOS
				uint32_t packetIndex = AtomicAddU32(&reader.readPacketCount, 1);
				ConsoleFormatOut("Read packet %lu\n", packetIndex);
#endif

				// Check if packet is in play range, then queue, otherwise discard
				int64_t streamStartTime = formatCtx->streams[srcPacket.stream_index]->start_time;
				int64_t pktTimeStamp = (srcPacket.pts == AV_NOPTS_VALUE) ? srcPacket.dts : srcPacket.pts;
				double timeInSeconds = (double)(pktTimeStamp - (streamStartTime != AV_NOPTS_VALUE ? streamStartTime : 0)) * av_q2d(formatCtx->streams[srcPacket.stream_index]->time_base);
				bool pktInPlayRange = (!state->settings.duration.isValid) ||
					((timeInSeconds / (double)AV_TIME_BASE) <= ((double)state->settings.duration.value / (double)AV_TIME_BASE));

				if ((videoStream != nullptr) && (srcPacket.stream_index == videoStream->streamIndex) && pktInPlayRange) {
					AddPacketToDecoder(video.decoder, targetPacket, &srcPacket);
#if PRINT_QUEUE_INFOS
					ConsoleFormatOut("Queued video packet %lu\n", packetIndex);
#endif
				} else if ((audioStream != nullptr) && (srcPacket.stream_index == audioStream->streamIndex) && pktInPlayRange) {
					AddPacketToDecoder(audio.decoder, targetPacket, &srcPacket);
#if PRINT_QUEUE_INFOS
					ConsoleFormatOut("Queued audio packet %lu\n", packetIndex);
#endif
				} else {
#if PRINT_QUEUE_INFOS
					ConsoleFormatOut("Dropped packet %lu\n", packetIndex);
#endif
					ffmpeg.avPacketUnref(&srcPacket);
				}
				hasPendingPacket = false;
			}
			skipWait = true;
		}
	}

	ConsoleOut("Reader thread stopped.\n");
}

static bool OpenStreamComponent(const char *mediaFilePath, AVStream *stream, MediaStream &outStream) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	// Get codec name
	char codecName[5] = {};
	MemoryCopy(&stream->codecpar->codec_tag, 4, codecName);

	// Determine codec type name
	const char *typeName;
	switch (stream->codecpar->codec_type) {
		case AVMEDIA_TYPE_VIDEO:
			typeName = "Video";
			break;
		case AVMEDIA_TYPE_AUDIO:
			typeName = "Audio";
			break;
		default:
			assert(!"Unsupported stream type!");
	}

	// Create codec context
	outStream.codecContext = ffmpeg.avcodecAllocContext3(nullptr);
	if (ffmpeg.avcodecParametersToContext(outStream.codecContext, stream->codecpar) < 0) {
		ConsoleFormatError("Failed getting %s codec context from codec '%s' in media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// @NOTE(finaL): Set packet time base to stream time base
	outStream.codecContext->pkt_timebase = stream->time_base;

	// Find decoder
	// @TODO(final): We could force the codec here if we want (avcodec_find_decoder_by_name).
	outStream.codec = ffmpeg.avcodecFindDecoder(stream->codecpar->codec_id);
	if (outStream.codec == nullptr) {
		ConsoleFormatError("Unsupported %s codec '%s' in media file '%s' found!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// Open codec
	if (ffmpeg.avcodecOpen2(outStream.codecContext, outStream.codec, nullptr) < 0) {
		ConsoleFormatError("Failed opening %s codec '%s' from media file '%s'!\n", typeName, codecName, mediaFilePath);
		return false;
	}

	// @TODO(final): Why do we need to set the discard flag to default here?
	stream->discard = AVDISCARD_DEFAULT;

	outStream.isValid = true;
	outStream.stream = stream;

	return true;
}

static bool IsRealTime(AVFormatContext *s) {
	if (!strcmp(s->iformat->name, "rtp") ||
		!strcmp(s->iformat->name, "rtsp") ||
		!strcmp(s->iformat->name, "sdp")) {
		return true;
	}
	if (s->pb && (!strncmp(s->filename, "rtp:", 4) || !strncmp(s->filename, "udp:", 4))) {
		return true;
	}
	return false;
}

struct RefreshState {
	double remainingTime;
};

static void DisplayVideoFrame(PlayerState *state) {
	assert(state != nullptr);
	Frame *vp = PeekFrameQueueLast(state->video.decoder.frameQueue);
	if (!vp->isUploaded) {
		VideoContext &video = state->video;
		UploadTexture(video, vp->frame);
	}
	WindowFlip();
}

inline void UpdateVideoClock(PlayerState *state, double pts, int32_t serial) {
	SetClock(state->video.clock, pts, serial);
	SyncClockToSlave(state->externalClock, state->video.clock);
}

inline double GetFrameDuration(PlayerState *state, const Frame *cur, const Frame *next) {
	if (cur->serial == next->serial) {
		double duration = next->pts - cur->pts;
		if (isnan(duration) || duration <= 0 || duration > state->maxFrameDuration)
			return cur->duration;
		else
			return duration;
	} else {
		return 0.0;
	}
}

static double ComputeVideoDelay(PlayerState *state, double delay) {
	if (GetMasterSyncType(state) != AVSyncType::VideoMaster) {
		double diff = GetClock(state->video.clock) - GetMasterClock(state);
		double syncThreshold = FPL_MAX(AV_SYNC_THRESHOLD_MIN, FPL_MIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < state->maxFrameDuration) {
			if (diff <= -syncThreshold) {
				delay = FFMAX(0, delay + diff);
			} else if (diff >= syncThreshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
				delay = delay + diff;
			} else if (diff >= syncThreshold) {
				delay = 2 * delay;
			}
		}
	}
	return(delay);
}

static void VideoRefresh(PlayerState *state, double *remainingTime) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;
	if (!state->isPaused && GetMasterSyncType(state) == AVSyncType::ExternalClock && state->isRealTime) {
		UpdateExternalClockSpeed(state);
	}
	if (state->video.stream.isValid) {
	retry:
		if (GetFrameQueueRemainingCount(state->video.decoder.frameQueue) == 0) {
			// Nothing to do, no picture to display in the queue
		} else {
			// Dequeue the current and the last picture
			Frame *lastvp = PeekFrameQueueLast(state->video.decoder.frameQueue);
			Frame *vp = PeekFrameQueue(state->video.decoder.frameQueue);

			// Serials from frame and packet queue must match
			if (vp->serial != state->video.decoder.packetsQueue.serial) {
				NextReadable(state->video.decoder.frameQueue);
				goto retry;
			}

			// Reset frame timer when serial from current and last frame as different
			if (lastvp->serial != vp->serial) {
				state->frameTimer = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
			}

			// Just display the current frame which was last shown
			if (state->isPaused) {
				goto display;
			}

			// compute nominal last_duration */
			double lastDuration = GetFrameDuration(state, lastvp, vp);
			double delay = ComputeVideoDelay(state, lastDuration);

			double time = (double)ffmpeg.avGetTimeRelative() / (double)AV_TIME_BASE;
			if (time < state->frameTimer + delay) {
				*remainingTime = FPL_MIN(state->frameTimer + delay - time, *remainingTime);
				return;
			}

			state->frameTimer += delay;
			if (delay > 0 && time - state->frameTimer > AV_SYNC_THRESHOLD_MAX) {
				state->frameTimer = time;
			}

			MutexLock(state->video.decoder.frameQueue.lock);
			if (!isnan(vp->pts)) {
				UpdateVideoClock(state, vp->pts, vp->serial);
			}
			MutexUnlock(state->video.decoder.frameQueue.lock);

			if (GetFrameQueueRemainingCount(state->video.decoder.frameQueue) > 1) {
				Frame *nextvp = PeekFrameQueueNext(state->video.decoder.frameQueue);
				double duration = GetFrameDuration(state, vp, nextvp);
				if (!state->step && (state->settings.frameDrop > 0 || (state->settings.frameDrop && GetMasterSyncType(state) != AVSyncType::VideoMaster)) && time > state->frameTimer + duration) {
					state->frame_drops_late++;
					NextReadable(state->video.decoder.frameQueue);
					goto retry;
				}
			}

			NextReadable(state->video.decoder.frameQueue);
			state->forceRefresh = 1;

			if (state->step && !state->isPaused) {
				StreamTogglePause(state);
			}
		}
	display:
		if (state->forceRefresh && state->video.decoder.frameQueue.readIndexShown) {
			DisplayVideoFrame(state);
		}
	}
	state->forceRefresh = 0;
}

static void ReleaseMedia(PlayerState &state) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	DestroyDecoder(state.audio.decoder);
	if (state.audio.conversionAudioBuffer != nullptr) {
		memory::MemoryAlignedFree(state.audio.conversionAudioBuffer);
	}
	if (state.audio.softwareResampleCtx != nullptr) {
		ffmpeg.swrFree(&state.audio.softwareResampleCtx);
	}
	if (state.audio.stream.codecContext != nullptr) {
		ffmpeg.avcodecFreeContext(&state.audio.stream.codecContext);
	}

	DestroyDecoder(state.video.decoder);
	if (state.video.softwareScaleCtx != nullptr) {
		ffmpeg.swsFreeContext(state.video.softwareScaleCtx);
	}
	if (state.video.targetRGBBuffer != nullptr) {
		MemoryAlignedFree(state.video.targetRGBBuffer);
	}
	if (state.video.targetRGBFrame != nullptr) {
		ffmpeg.avFrameFree(&state.video.targetRGBFrame);
	}
	if (state.video.targetTexture.id) {
		DestroyTexture(state.video.targetTexture);
	}
	if (state.video.stream.codecContext != nullptr) {
		ffmpeg.avcodecFreeContext(&state.video.stream.codecContext);
	}

	DestroyReader(state.reader);
	if (state.formatCtx != nullptr) {
		ffmpeg.avformatCloseInput(&state.formatCtx);
	}
}

static bool LoadMedia(PlayerState &state, const char *mediaFilePath, const AudioDeviceFormat &nativeAudioFormat, VideoBackBuffer *backBuffer) {
	const FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

	// @TODO(final): Custom IO!

	// Open media file
	if (ffmpeg.avformatOpenInput(&state.formatCtx, mediaFilePath, nullptr, nullptr) != 0) {
		ConsoleFormatError("Failed opening media file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Retrieve stream information
	if (ffmpeg.avformatFindStreamInfo(state.formatCtx, nullptr) < 0) {
		ConsoleFormatError("Failed getting stream informations for media file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Dump information about file onto standard error
	ffmpeg.avDumpFormat(state.formatCtx, 0, mediaFilePath, 0);

	// Dont limit the queues when we are playing realtime based media, like internet streams, etc.
	state.isRealTime = IsRealTime(state.formatCtx);
	if (!state.isInfiniteBuffer && state.isRealTime) {
		state.isInfiniteBuffer = true;
	}

	// Find the first streams
	state.video.stream.streamIndex = -1;
	state.audio.stream.streamIndex = -1;
	for (uint32_t steamIndex = 0; steamIndex < state.formatCtx->nb_streams; steamIndex++) {
		AVStream *stream = state.formatCtx->streams[steamIndex];
		switch (stream->codecpar->codec_type) {
			case AVMEDIA_TYPE_VIDEO:
			{
				if (state.video.stream.streamIndex == -1) {
					if (OpenStreamComponent(mediaFilePath, stream, state.video.stream)) {
						state.video.stream.streamIndex = steamIndex;
					}
				}
			} break;
			case AVMEDIA_TYPE_AUDIO:
			{
				if (state.audio.stream.streamIndex == -1) {
					if (OpenStreamComponent(mediaFilePath, stream, state.audio.stream)) {
						state.audio.stream.streamIndex = steamIndex;
					}
				}
			} break;
		}
	}

	// No streams found
	if ((!state.video.stream.isValid) && (!state.audio.stream.isValid)) {
		ConsoleFormatError("No video or audio stream in media file '%s' found!\n", mediaFilePath);
		goto release;
	}

	if (!InitReader(state.reader)) {
		ConsoleFormatError("Failed initializing reader file '%s'!\n", mediaFilePath);
		goto release;
	}

	// Allocate audio related resources
	if (state.audio.stream.isValid) {
		AudioContext &audio = state.audio;
		AVCodecContext *audioCodexCtx = audio.stream.codecContext;

		// Init audio decoder
		if (!InitDecoder(audio.decoder, &state, &state.reader, &audio.stream, MAX_AUDIO_FRAME_QUEUE_COUNT, 1)) {
			ConsoleFormatError("Failed initialize audio decoder for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// @TODO(final): Map target audio format to FFMPEG
		assert(nativeAudioFormat.type == AudioFormatType::S16);
		AVSampleFormat targetSampleFormat = AV_SAMPLE_FMT_S16;
		int targetChannelCount = nativeAudioFormat.channels;
		// @TODO(final): Map target audio channels to channel layout
		uint64_t targetChannelLayout = AV_CH_LAYOUT_STEREO;
		assert(targetChannelCount == 2);
		int targetSampleRate = nativeAudioFormat.sampleRate;

		AVSampleFormat inputSampleFormat = audioCodexCtx->sample_fmt;
		int inputChannelCount = audioCodexCtx->channels;
		// @TODO(final): Map input audio channels to channel layout
		uint64_t inputChannelLayout = AV_CH_LAYOUT_STEREO;
		int inputSampleRate = audioCodexCtx->sample_rate;
		assert(inputChannelCount == 2);

		// Create software resample context and initialize
		audio.softwareResampleCtx = ffmpeg.swrAllocSetOpts(nullptr,
														   targetChannelLayout,
														   targetSampleFormat,
														   targetSampleRate,
														   inputChannelLayout,
														   inputSampleFormat,
														   inputSampleRate,
														   0,
														   nullptr);
		ffmpeg.swrInit(audio.softwareResampleCtx);

		// Allocate conversion buffer in native format, this must be big enough to hold one AVFrame worth of data.
		int lineSize;
		audio.maxConversionAudioBufferSize = ffmpeg.avSamplesGetBufferSize(&lineSize, targetChannelCount, targetSampleRate, targetSampleFormat, 1);
		audio.maxConversionAudioFrameCount = audio.maxConversionAudioBufferSize / audio::GetAudioSampleSizeInBytes(nativeAudioFormat.type) / targetChannelCount;
		audio.conversionAudioBuffer = (uint8_t *)memory::MemoryAlignedAllocate(audio.maxConversionAudioBufferSize, 16);
		audio.conversionAudioFrameIndex = 0;
		audio.conversionAudioFramesRemaining = 0;
	}

	// Allocate video related resources
	if (state.video.stream.isValid) {
		VideoContext &video = state.video;
		AVCodecContext *videoCodexCtx = video.stream.codecContext;

		// Init video decoder
		if (!InitDecoder(video.decoder, &state, &state.reader, &video.stream, MAX_VIDEO_FRAME_QUEUE_COUNT, 1)) {
			ConsoleFormatError("Failed initialize video decoder for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Allocate RGB video frame
		video.targetRGBFrame = ffmpeg.avFrameAlloc();
		if (video.targetRGBFrame == nullptr) {
			ConsoleFormatError("Failed allocating RGB video frame for media file '%s'!\n", mediaFilePath);
			goto release;
		}

		// Allocate RGB buffer
		AVPixelFormat targetPixelFormat = AVPixelFormat::AV_PIX_FMT_BGR24;
		size_t rgbFrameSize = ffmpeg.avImageGetBufferSize(targetPixelFormat, videoCodexCtx->width, videoCodexCtx->height, 1);
		video.targetRGBBuffer = (uint8_t *)MemoryAlignedAllocate(rgbFrameSize, 16);

		// Setup RGB video frame and give it access to the actual data
		ffmpeg.avImageFillArrays(video.targetRGBFrame->data, video.targetRGBFrame->linesize, video.targetRGBBuffer, targetPixelFormat, videoCodexCtx->width, videoCodexCtx->height, 1);

		// Get software context
		video.softwareScaleCtx = ffmpeg.swsGetContext(
			videoCodexCtx->width,
			videoCodexCtx->height,
			videoCodexCtx->pix_fmt,
			videoCodexCtx->width,
			videoCodexCtx->height,
			targetPixelFormat,
			SWS_BILINEAR,
			nullptr,
			nullptr,
			nullptr
		);
		if (video.softwareScaleCtx == nullptr) {
			ConsoleFormatError("Failed getting software scale context with size (%d x %d) for file '%s'!\n", videoCodexCtx->width, videoCodexCtx->height, mediaFilePath);
			goto release;
		}

		// Resize backbuffer to fit the video size
		if (!ResizeVideoBackBuffer(videoCodexCtx->width, videoCodexCtx->height)) {
			ConsoleFormatError("Failed resizing video backbuffer to size (%d x %d) for file '%s'!\n", videoCodexCtx->width, videoCodexCtx->height, mediaFilePath);
			goto release;
		}

		if (!InitTexture(state.video.targetTexture, backBuffer->width, backBuffer->height, 32, (uint8_t *)backBuffer->pixels)) {
			goto release;
		}

		state.frameTimer = 0.0;
		state.frameLastPTS = 0.0;
		state.frameLastDelay = 40e-3;
	}

	// Init timings
	state.maxFrameDuration = (state.formatCtx->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;
	InitClock(state.video.clock, &state.video.decoder.packetsQueue.serial);
	InitClock(state.audio.clock, &state.audio.decoder.packetsQueue.serial);
	InitClock(state.externalClock, &state.externalClock.serial);
	state.audio.audioClockSerial = -1;

	return true;

release:
	ReleaseMedia(state);
	return false;
}

int main(int argc, char **argv) {
	int result = 0;

	if (argc < 2) {
		ConsoleError("Media file argument missing!");
		return -1;
	}

	const char *mediaFilePath = argv[1];

	Settings settings = DefaultSettings();
	CopyAnsiString("FPL FFmpeg Demo", settings.window.windowTitle, FPL_ARRAYCOUNT(settings.window.windowTitle));
	settings.video.driverType = VideoDriverType::Software;
	settings.video.isAutoSize = false;
	settings.video.isVSync = true;

	if (InitPlatform(InitFlags::All, settings)) {
		VideoBackBuffer *backBuffer = GetVideoBackBuffer();
		AudioDeviceFormat nativeAudioFormat = GetAudioHardwareFormat();

		globalFFMPEGFunctions = (FFMPEGContext *)MemoryAlignedAllocate(sizeof(FFMPEGContext), 16);
		FFMPEGContext &ffmpeg = *globalFFMPEGFunctions;

		PlayerState state = {};
		RefreshState refresh = {};

		//
		// Load ffmpeg libraries
		//
		if (!LoadFFMPEG(ffmpeg)) {
			goto release;
		}

		// Register all formats and codecs
		ffmpeg.avRegisterAll();

		// Init flush packet
		ffmpeg.avInitPacket(&globalFlushPacket);
		globalFlushPacket.data = (uint8_t *)&globalFlushPacket;

		//
		// Settings
		//
		InitPlayerSettings(state.settings);
		state.isInfiniteBuffer = state.settings.isInfiniteBuffer;
		state.loop = state.settings.isLoop ? 1 : 0;

		// Load media
		if (!LoadMedia(state, mediaFilePath, nativeAudioFormat, backBuffer)) {
			goto release;
		}

		// Start decoder and reader
		if (state.video.stream.isValid) {
			StartDecoder(state.video.decoder, VideoDecodingThreadProc);
		}
		if (state.audio.stream.isValid) {
			StartDecoder(state.audio.decoder, AudioDecodingThreadProc);
		}
		StartReader(state.reader, PacketReadThreadProc, &state);

		// Start playing audio
		if (state.audio.stream.isValid) {
			SetAudioClientReadCallback(AudioReadCallback, &state.audio);
			PlayAudio();
		}

		//
		// App loop
		//
		double lastTime = GetHighResolutionTimeInSeconds();
		double remainingTime = 0.0;
		while (WindowUpdate()) {
			//
			// Handle events
			//
			Event ev = {};
			while (PollWindowEvent(ev)) {
				if (ev.type == EventType::Keyboard) {
					if ((ev.keyboard.type == KeyboardEventType::KeyUp) && (ev.keyboard.mappedKey == Key::Key_Space)) {
						TogglePause(&state);
					}
				}
			}

			// Refresh video?
			if (remainingTime <= 0.0) {
				remainingTime = DEFAULT_REFRESH_RATE;
				if (!state.isPaused || state.forceRefresh) {
					VideoRefresh(&state, &remainingTime);
				}
			}

			// Update time
			double now = GetHighResolutionTimeInSeconds();
			double delta = now - lastTime;
			lastTime = now;
			remainingTime -= delta;
			PrintMemStats();
		}


	release:
		// Stop audio
		if (state.audio.stream.isValid) {
			StopAudio();
		}

		// Stop reader and decoders
		StopReader(state.reader);
		if (state.video.stream.isValid) {
			StopDecoder(state.video.decoder);
		}
		if (state.audio.stream.isValid) {
			StopDecoder(state.audio.decoder);
		}

		// Release media
		ReleaseMedia(state);

		//
		// Release FFMPEG
		//
		ReleaseFFMPEG(ffmpeg);
		MemoryAlignedFree(globalFFMPEGFunctions);

		// Release platform
		ReleasePlatform();

		result = 0;
	} else {
		result = -1;
	}

	return(result);
}