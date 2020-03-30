#ifndef _ELASTOS_ERRCODE_HPP_
#define _ELASTOS_ERRCODE_HPP_

#include <string>
#include <functional>

namespace elastos {

class ErrCode {
public:
    /*** type define ***/
#define CHECK_ERROR(errCode) \
	if((errCode) < 0) { \
	    int errRet = (errCode); \
		APPEND_SRCLINE(errRet); \
		Log::E(Log::TAG, "Failed to call %s in line %d, return %d.", FORMAT_METHOD, __LINE__, errRet); \
		return errRet; \
	}

#define CHECK_RETVAL(errCode) \
	if(errCode < 0) { \
	    int errRet = errCode; \
		APPEND_SRCLINE(errRet); \
		Log::E(Log::TAG, "Failed to call %s in line %d, return %d.", FORMAT_METHOD, __LINE__, errRet); \
        return; \
	}

#define CHECK_AND_RETDEF(errCode, def) \
	if(errCode < 0) { \
	    int errRet = errCode; \
		APPEND_SRCLINE(errRet); \
		Log::E(Log::TAG, "Failed to call %s in line %d, return %d.", FORMAT_METHOD, __LINE__, errRet); \
		return def; \
	}

#define CHECK_AND_NOTIFY_ERROR(errCode) \
	if(errCode < 0) { \
	    int errRet = errCode; \
		APPEND_SRCLINE(errRet); \
		elastos::ErrCode::SetError(errRet, std::string(FORMAT_METHOD) + " line:" + std::to_string(__LINE__)); \
	} \
    CHECK_ERROR(errCode) \

#define CHECK_AND_NOTIFY_RETVAL(errCode) \
	if(errCode < 0) { \
	    int errRet = errCode; \
		APPEND_SRCLINE(errRet); \
		elastos::ErrCode::SetError(errRet, std::string(FORMAT_METHOD) + " line:" + std::to_string(__LINE__)); \
	} \
    CHECK_RETVAL(errCode) \

#define APPEND_SRCLINE(errRet) \
	if(errRet > elastos::ErrCode::SourceLineSection) {              \
		errRet += (__LINE__ * elastos::ErrCode::SourceLineSection); \
	}                                                               \

#define GET_ERRCODE(errRet) \
	(errRet - errRet / elastos::ErrCode::SourceLineSection * elastos::ErrCode::SourceLineSection)

    /*** static function and variable ***/
    constexpr static const int UnknownError = -1;
    constexpr static const int UnimplementedError = -2;
    constexpr static const int NotFoundError = -3;
    constexpr static const int NotReadyError = -4;
    constexpr static const int InvalidArgument = -5;
    constexpr static const int IOSystemException = -6;
    constexpr static const int NetworkException = -7;
    constexpr static const int PointerReleasedError = -8;
    constexpr static const int DevUUIDError = -9;
    constexpr static const int FileNotExistsError = -10;
    constexpr static const int JsonParseException = -11;
    constexpr static const int ConflictWithExpectedError = -12;
    constexpr static const int MergeInfoFailed = -13;
    constexpr static const int IgnoreMergeOldInfo = -14;
    constexpr static const int EmptyInfoError = -15;
    constexpr static const int InvalidFriendCode = -16;
    constexpr static const int RepeatOperationError = -17;
	constexpr static const int CreateDirectoryError = -18;
    constexpr static const int ExpectedBeforeStartedError = -19;
	constexpr static const int ExpectedAfterStartedError = -20;
	constexpr static const int SizeOverflowError = -21;
	constexpr static const int NotExpectedReachedError = -22;

    constexpr static const int InvalidLocalDataDir = -50;
    constexpr static const int NoSecurityListener = -51;
    constexpr static const int BadSecurityValue = -52;
    constexpr static const int KeypairError = -53;

    constexpr static const int ChannelFailedPresetAll = -200;
    constexpr static const int ChannelFailedOpenAll = -201;
    constexpr static const int ChannelFailedCloseAll = -202;
    constexpr static const int ChannelFailedMultiOpen = -203;
    constexpr static const int ChannelFailedCarrier = -204;
    constexpr static const int ChannelFailedFriendExists = -205;
    constexpr static const int ChannelFailedFriendSelf = -206;
    constexpr static const int ChannelNotFound = -207;
    constexpr static const int ChannelNotReady = -208;
    constexpr static const int ChannelNotEstablished = -209;
    constexpr static const int ChannelNotOnline = -210;
    constexpr static const int ChannelNotSendMessage = -211;
    constexpr static const int ChannelDataTooLarge = -212;
	constexpr static const int ChannelFailedFileTrans = -213;
	constexpr static const int ChannelFileTransBusy = -214;
	constexpr static const int ChannelFailedReadData = -215;
	constexpr static const int ChannelInvalidDataId = -216;

    constexpr static const int BlkChnSetPropError = -300;
    constexpr static const int BlkChnGetPropError = -301;
    constexpr static const int BlkChnEmptyPropError = -302;
    constexpr static const int BlkChnBadTxIdError = -303;
    constexpr static const int BlkChnBadUpdateTimeError = -304;
    constexpr static const int BlkChnOldUpdateTimeError = -305;
    constexpr static const int BlkChnMonStoppedError = -306;
	constexpr static const int BlkChnIgnoreCacheProp = -307;

	constexpr static const int ProofApiSetPropError = -320;
	constexpr static const int ProofApiGetPropError = -321;
	constexpr static const int ProofApiEmptyPropError = -322;
	constexpr static const int ProofApiCheckAuthHashError = -323;

	constexpr static const int AdditivityIndex = -1000;
	constexpr static const int StdSystemErrorIndex = -1000;
	constexpr static const int HttpClientErrorIndex = -2000;
	constexpr static const int RemoteStorageClientErrorIndex = -3000;

	constexpr static const int SourceLineSection = -100000;

	static void SetErrorListener(std::function<void(int, const std::string&, const std::string&)> listener);
	static void SetError(int errCode, const std::string& ext);
    static std::string ToString(int errCode);

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
	static std::function<void(int, const std::string&, const std::string&)> sErrorListener;

    /*** class function and variable ***/
    explicit ErrCode() = delete;
    virtual ~ErrCode() = delete;

}; // class ErrCode

} // namespace elastos

#endif /* _ELASTOS_ERRCODE_HPP_ */
