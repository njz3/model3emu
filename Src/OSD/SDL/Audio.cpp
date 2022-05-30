/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

 /*
  * Audio.cpp
  *
  * SDL audio playback. Implements the OSD audio interface.
  *
  * Buffer sizes and read/write positions must be sample-aligned. A sample is
  * defined to encompass both channels so for, e.g., 16-bit audio as used here,
  * a sample is 4 bytes. Static assertions are employed to ensure that the
  * initial set up of the buffer is correct.
  *
  * Model 3 Audio is always 4 channels (one SCSP for each front/rear output).
  * The downmix to 2 channels will be performed here in case supermodel audio
  * subsystem does not allow such playback.
  * The default DSB board is supposed to be plug and mixed with the rear output
  * channel. This rear channel is usually plugged to the gullbow speakers that
  * are present in all racing cabinets, while front speakers are only present
  * in Daytona2, Scud Race, Sega Rally 2.
  */

#include "Audio.h"

#include "Supermodel.h"
#include "SDLIncludes.h"

#include <cmath>
#include <algorithm>

  // Model3 audio output is 44.1KHz 4-channel sound and frame rate is 60fps
#define SAMPLE_RATE_M3     (44100)
#define SUPERMODEL_FPS     (60.0f)
#define MODEL3_FPS         (57.53f)

#define MAX_SND_FREQ       (75)
#define MIN_SND_FREQ       (45)
#define MAX_LATENCY        (100)

#define NUM_CHANNELS_M3 (4)

int nbHostAudioChannels = NUM_CHANNELS_M3;   // Number of channels on host

#define SAMPLES_PER_FRAME_M3  (INT32)(SAMPLE_RATE_M3 / MODEL3_FPS)
 
#define BYTES_PER_SAMPLE_M3   (NUM_CHANNELS_M3 * sizeof(INT16))
#define BYTES_PER_FRAME_M3   (SAMPLES_PER_FRAME_M3 * BYTES_PER_SAMPLE_M3)


static int samples_per_frame_host = SAMPLES_PER_FRAME_M3;
static int bytes_per_sample_host = BYTES_PER_SAMPLE_M3;
static int bytes_per_frame_host = BYTES_PER_FRAME_M3;

// Balance percents for mixer
float BalanceLeftRight = 0; // 0 mid balance, 100: left only,  -100:right only 
float BalanceFrontRear = 0; // 0 mid balance, 100: front only, -100:right only 
// Mixer factor (depends on values above)
float balanceFactorFrontLeft  = 1.0;
float balanceFactorFrontRight = 1.0;
float balanceFactorRearLeft   = 1.0;
float balanceFactorRearRight  = 1.0;

static bool enabled = true;         // True if sound output is enabled
static constexpr unsigned latency = 20;       // Audio latency to use (ie size of audio buffer) as percentage of max buffer size
static constexpr bool underRunLoop = true;    // True if should loop back to beginning of buffer on under-run, otherwise sound is just skipped

static constexpr unsigned playSamples = 512;  // Size (in samples) of callback play buffer

static UINT32 audioBufferSize = 0;  // Size (in bytes) of audio buffer
static INT8* audioBuffer = NULL;    // Audio buffer

static UINT32 writePos = 0;         // Current position at which writing into buffer
static UINT32 playPos = 0;          // Current position at which playing data in buffer via callback

static bool writeWrapped = false;   // True if write position has wrapped around at end of buffer but play position has not done so yet

static unsigned underRuns = 0;      // Number of buffer under-runs that have occured
static unsigned overRuns = 0;       // Number of buffer over-runs that have occured

static AudioCallbackFPtr callback = NULL; // Pointer to audio callback that is called when audio buffer is less than half empty
static void* callbackData = NULL;         // Pointer to data to be passed to audio callback when it is called

static const Util::Config::Node* s_config = 0;


void SetAudioCallback(AudioCallbackFPtr newCallback, void* newData)
{
    // Lock audio whilst changing callback pointers
    SDL_LockAudio();

    callback = newCallback;
    callbackData = newData;

    SDL_UnlockAudio();
}

void SetAudioEnabled(bool newEnabled)
{
    enabled = newEnabled;
}

static void PlayCallback(void* data, Uint8* stream, int len)
{
    //printf("PlayCallback(%d) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u]\n",
    //	len, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize);

    // Get current write position and adjust it if write has wrapped but play position has not
    UINT32 adjWritePos = writePos;
    if (writeWrapped)
        adjWritePos += audioBufferSize;

    // Check if play position overlaps write position (ie buffer under-run)
    if (playPos + len > adjWritePos) {
        underRuns++;

        //printf("Audio buffer under-run #%u in PlayCallback(%d) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u]\n",
        //	underRuns, len, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize);

        // See what action to take on under-run
        if (underRunLoop) {
            // If loop, then move play position back to beginning of data in buffer
            playPos = adjWritePos + bytes_per_frame_host;

            // Check if play position has moved past end of buffer
            if (playPos >= audioBufferSize)
                // If so, wrap it around to beginning again (but keep write wrapped flag as before)
                playPos -= audioBufferSize;
            else
                // Otherwise, set write wrapped flag as will now appear as if write has wrapped but play position has not
                writeWrapped = true;
        } else {
            // Otherwise, just copy silence to audio output stream and exit
            memset(stream, 0, len);
            return;
        }
    }

    INT8* src1;
    INT8* src2;
    UINT32 len1;
    UINT32 len2;

    // Check if play region extends past end of buffer
    if (playPos + len > audioBufferSize) {
        // If so, split play region into two
        src1 = audioBuffer + playPos;
        src2 = audioBuffer;
        len1 = audioBufferSize - playPos;
        len2 = len - len1;
    } else {
        // Otherwise, just copy whole region
        src1 = audioBuffer + playPos;
        src2 = 0;
        len1 = len;
        len2 = 0;
    }

    // Check if audio is enabled
    if (enabled) {
        // If so, copy play region into audio output stream
        memcpy(stream, src1, len1);

        // Also, if not looping on under-runs then blank region out
        if (!underRunLoop)
            memset(src1, 0, len1);

        if (len2) {
            // If region was split into two, copy second half into audio output stream as well
            memcpy(stream + len1, src2, len2);

            // Also, if not looping on under-runs then blank region out
            if (!underRunLoop)
                memset(src2, 0, len2);
        }
    } else
        // Otherwise, just copy silence to audio output stream
        memset(stream, 0, len);

    // Move play position forward for next time
    playPos += len;

    bool bufferFull = adjWritePos + 2 * bytes_per_frame_host > playPos + audioBufferSize;

    // Check if play position has moved past end of buffer
    if (playPos >= audioBufferSize) {
        // If so, wrap it around to beginning again and reset write wrapped flag
        playPos -= audioBufferSize;
        writeWrapped = false;
    }

    // If buffer is not full then call audio callback
    if (callback && !bufferFull)
        callback(callbackData);
}

static void MixChannels(unsigned numSamples, INT16* leftFrontBuffer, INT16* rightFrontBuffer, INT16* leftRearBuffer, INT16* rightRearBuffer, void* dest, bool flipStereo)
{
    INT16* p = (INT16*)dest;


    if (nbHostAudioChannels == 1) {
        for (unsigned i = 0; i < numSamples; i++) {
            // TODO: these should probably be clipped!
            //INT16 monovalue = (INT16)(((INT32)leftFrontBuffer[i] + (INT32)rightFrontBuffer[i] + (INT32)leftRearBuffer[i] + (INT32)rightRearBuffer[i]) >> 2);
            INT16 monovalue = (INT16)(((INT32)(leftFrontBuffer[i]*balanceFactorFrontLeft) + (INT32)(rightFrontBuffer[i]*balanceFactorFrontRight) + (INT32)(leftRearBuffer[i]*balanceFactorRearLeft) + (INT32)(rightRearBuffer[i]*balanceFactorRearRight)) >> 2);
            *p++ = monovalue;
        }
    } else {
        if (flipStereo) // swap left and right channels
        {
            if (nbHostAudioChannels == 2) {
                for (unsigned i = 0; i < numSamples; i++) {
                    INT16 leftvalue = (INT16)(((INT32)(leftFrontBuffer[i]*balanceFactorFrontLeft) + (INT32)(leftRearBuffer[i]*balanceFactorRearLeft))>>1);
                    INT16 rightvalue = (INT16)(((INT32)(rightFrontBuffer[i]*balanceFactorFrontRight) + (INT32)(rightRearBuffer[i]*balanceFactorRearRight))>>1);
                    *p++ = rightvalue;
                    *p++ = leftvalue;
                }
            } else if (nbHostAudioChannels == 4) {
                for (unsigned i = 0; i < numSamples; i++) {
                    *p++ = rightFrontBuffer[i]*balanceFactorFrontRight;
                    *p++ = leftFrontBuffer[i]*balanceFactorFrontLeft;
                    *p++ = rightRearBuffer[i]*balanceFactorRearRight;
                    *p++ = leftRearBuffer[i]*balanceFactorRearLeft;
                }
            }
        } else {
            // correct stereo
            if (nbHostAudioChannels == 2) {
                for (unsigned i = 0; i < numSamples; i++) {
                    INT16 leftvalue = (INT16)(((INT32)(leftFrontBuffer[i]*balanceFactorFrontLeft) + (INT32)(leftRearBuffer[i]*balanceFactorRearLeft))>>1);
                    INT16 rightvalue = (INT16)(((INT32)(rightFrontBuffer[i]*balanceFactorFrontRight) + (INT32)(rightRearBuffer[i]*balanceFactorRearRight))>>1);
                    *p++ = leftvalue;
                    *p++ = rightvalue;
                }
            } else if (nbHostAudioChannels == 4) {
                for (unsigned i = 0; i < numSamples; i++) {
                    *p++ = leftFrontBuffer[i]*balanceFactorFrontLeft;
                    *p++ = rightFrontBuffer[i]*balanceFactorFrontRight;
                    *p++ = leftRearBuffer[i]*balanceFactorRearLeft;
                    *p++ = rightRearBuffer[i]*balanceFactorRearRight;
                }
            }
        }
    }
}

/*
static void LogAudioInfo(SDL_AudioSpec *fmt)
{
    InfoLog("Audio device information:");
    InfoLog("    Frequency: %d", fmt->freq);
    InfoLog("     Channels: %d", fmt->channels);
    InfoLog("Sample Format: %d", fmt->format);
    InfoLog("");
}
*/

bool OpenAudio(const Util::Config::Node& config)
{
    s_config = &config;
    // Initialize SDL audio sub-system
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
        return ErrorLog("Unable to initialize SDL audio sub-system: %s\n", SDL_GetError());
    
    // Number of host channels to use (choice limited to 1,2,4)
    int reqChannels = s_config->Get("NbSoundChannels").ValueAs<int>();
    switch (reqChannels) {
        case 1:
        case 2:
        case 4:
            nbHostAudioChannels = reqChannels;
            break;
    }
    // Mixer Balance
    float balancelr = (float)s_config->Get("BalanceLeftRight").ValueAs<float>();
    if (balancelr < -100.0f)
        balancelr = -100.0f;
    else if (balancelr > 100.0f)
        balancelr = 100.0f;
    balancelr *= 0.01f;
    BalanceLeftRight = balancelr;

    float balancefr = (float)s_config->Get("BalanceFrontRear").ValueAs<float>();
    if (balancefr < -100.0f)
        balancefr = -100.0f;
    else if (balancefr > 100.0f)
        balancefr = 100.0f;
    balancefr *= 0.01f;
    BalanceFrontRear = balancefr;

    balanceFactorFrontLeft  = (BalanceLeftRight<0.0?1.0+BalanceLeftRight:1.0)*(BalanceFrontRear<0?1.0+BalanceFrontRear:1.0);
    balanceFactorFrontRight = (BalanceLeftRight>0.0?1.0-BalanceLeftRight:1.0)*(BalanceFrontRear<0?1.0+BalanceFrontRear:1.0);
    balanceFactorRearLeft   = (BalanceLeftRight<0.0?1.0+BalanceLeftRight:1.0)*(BalanceFrontRear>0?1.0-BalanceFrontRear:1.0);
    balanceFactorRearRight  = (BalanceLeftRight>0.0?1.0-BalanceLeftRight:1.0)*(BalanceFrontRear>0?1.0-BalanceFrontRear:1.0);

    // Set up audio specification
    SDL_AudioSpec desired, obtained;
    memset(&desired, 0, sizeof(SDL_AudioSpec));
    memset(&obtained, 0, sizeof(SDL_AudioSpec));
    desired.freq = SAMPLE_RATE_M3;
    desired.channels = nbHostAudioChannels;
    desired.format = AUDIO_S16SYS;
    desired.samples = playSamples;
    desired.callback = PlayCallback;

    // Force SDL to use the format we requested; it will convert if necessary
    //if (SDL_OpenAudio(&desired, &obtained) < 0) {
   	if (SDL_OpenAudio(&desired, nullptr) < 0) {
        if (NUM_CHANNELS_M3==2) {
            return ErrorLog("Unable to open 44.1KHz 2-channel audio with SDL: %s\n", SDL_GetError());
        } else if (NUM_CHANNELS_M3==4) {
            return ErrorLog("Unable to open 44.1KHz 4-channel audio with SDL: %s\n", SDL_GetError());
        } else {
            return ErrorLog("Unable to open 44.1KHz channel audio with SDL: %s\n", SDL_GetError());
        }
    } else {
        char buff[255];
        sprintf(buff, "SDL Audio opened with %d channels (max %d channels)\n", nbHostAudioChannels, ((int)NUM_CHANNELS_M3));
        nbHostAudioChannels = desired.channels;
        DebugLog(buff);
    }
    
    
    float soundFreq_Hz = s_config->Get("SoundFreq").ValueAs<float>();
    if (soundFreq_Hz>MAX_SND_FREQ)
        soundFreq_Hz = MAX_SND_FREQ;
    if (soundFreq_Hz<MIN_SND_FREQ)
        soundFreq_Hz = MIN_SND_FREQ;
    samples_per_frame_host = (INT32)(SAMPLE_RATE_M3 / soundFreq_Hz);
    bytes_per_sample_host = (nbHostAudioChannels * sizeof(INT16));
    bytes_per_frame_host =  (samples_per_frame_host * bytes_per_sample_host);

    // Put new code in condition pair until stable
#if true

    // Create audio buffer
    uint32_t bufferSize = ((SAMPLE_RATE_M3 * latency) / MAX_LATENCY) * bytes_per_sample_host;
    //static_assert(bufferSize % BYTES_PER_SAMPLE == 0, "must be an integer multiple of the sample size");
    if (!(bufferSize % bytes_per_sample_host == 0)) {
        return ErrorLog("must be an integer multiple of the sample size\n");
    }
    audioBufferSize = bufferSize;

    int minBufferSize = 3 * bytes_per_frame_host;
    audioBufferSize = std::max<int>(minBufferSize, audioBufferSize);
    audioBuffer = new(std::nothrow) INT8[audioBufferSize];
    if (audioBuffer == NULL) {
        float audioBufMB = (float)audioBufferSize / (float)0x100000;
        return ErrorLog("Insufficient memory for audio latency buffer (need %1.1f MB).", audioBufMB);
    }
    memset(audioBuffer, 0, sizeof(INT8) * audioBufferSize);

    // Set initial play position to be beginning of buffer and initial write position to be half-way into buffer
    playPos = 0;
    uint32_t endOfBuffer = bufferSize - bytes_per_frame_host;
    uint32_t midpointAfterFirstFrameUnaligned = bytes_per_frame_host + (bufferSize - bytes_per_frame_host) / 2;
    uint32_t extraPaddingNeeded = (bytes_per_frame_host - midpointAfterFirstFrameUnaligned % bytes_per_frame_host) % bytes_per_frame_host;
    uint32_t midpointAfterFirstFrame = midpointAfterFirstFrameUnaligned + extraPaddingNeeded;
    if (!((endOfBuffer % (nbHostAudioChannels*sizeof(INT16))) == 0)) {
        return ErrorLog("must be an integer multiple of the sample size\n");
    }
    if (!((midpointAfterFirstFrame % nbHostAudioChannels*sizeof(INT16)) == 0)) {
        return ErrorLog("must be an integer multiple of the sample size\n");
    }
    //static_assert(endOfBuffer % BYTES_PER_SAMPLE == 0, "make sure we are aligned to a sample boundary otherwise underrun/overrun adjustment will end up shifting playback by one channel causing stereo to flip");
    //static_assert(midpointAfterFirstFrame % BYTES_PER_SAMPLE == 0, "error");

#else
    
    
    // Create audio buffer
    constexpr uint32_t bufferSize = SAMPLE_RATE * BYTES_PER_SAMPLE_M3 * latency / MAX_LATENCY;
    static_assert(bufferSize % BYTES_PER_SAMPLE_M3 == 0, "must be an integer multiple of the sample size");
    audioBufferSize = bufferSize;

    int minBufferSize = 3 * BYTES_PER_FRAME_M3;
    audioBufferSize = std::max<int>(minBufferSize, audioBufferSize);
    audioBuffer = new(std::nothrow) INT8[audioBufferSize];
    if (audioBuffer == NULL) {
        float audioBufMB = (float)audioBufferSize / (float)0x100000;
        return ErrorLog("Insufficient memory for audio latency buffer (need %1.1f MB).", audioBufMB);
    }
    memset(audioBuffer, 0, sizeof(INT8) * audioBufferSize);

    // Set initial play position to be beginning of buffer and initial write position to be half-way into buffer
    playPos = 0;
    constexpr uint32_t endOfBuffer = bufferSize - BYTES_PER_FRAME_M3;
    constexpr uint32_t midpointAfterFirstFrameUnaligned = BYTES_PER_FRAME_M3 + (bufferSize - BYTES_PER_FRAME_M3) / 2;
    constexpr uint32_t extraPaddingNeeded = (BYTES_PER_SAMPLE_M3 - midpointAfterFirstFrameUnaligned % BYTES_PER_SAMPLE_M3) % BYTES_PER_SAMPLE_M3;
    constexpr uint32_t midpointAfterFirstFrame = midpointAfterFirstFrameUnaligned + extraPaddingNeeded;
    static_assert(endOfBuffer % BYTES_PER_SAMPLE_M3 == 0, "make sure we are aligned to a sample boundary otherwise underrun/overrun adjustment will end up shifting playback by one channel causing stereo to flip");
    static_assert(midpointAfterFirstFrame % BYTES_PER_SAMPLE_M3 == 0, "error");

#endif
    
    writePos = std::min<int>(endOfBuffer, midpointAfterFirstFrame);
    writeWrapped = false;

    // Reset counters
    underRuns = 0;
    overRuns = 0;

    // Start audio playing
    SDL_PauseAudio(0);
    return OKAY;
}

//static INT16 mixBuffer[NUM_CHANNELS_M3 * SAMPLES_PER_FRAME];

bool OutputAudio(unsigned numSamples, INT16* leftFrontBuffer, INT16* rightFrontBuffer, INT16* leftRearBuffer, INT16* rightRearBuffer, bool flipStereo)
{
    //printf("OutputAudio(%u) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u]\n",
    //	numSamples, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize);

    UINT32 bytesRemaining;
    UINT32 bytesToCopy;
    INT16* src;

    // Number of samples should never be more than max number of samples per frame
    if (numSamples > samples_per_frame_host)
        numSamples = samples_per_frame_host;

    // Mix together left and right channels into single chunk of data
    INT16 mixBuffer[NUM_CHANNELS_M3 * (SAMPLE_RATE_M3 / MIN_SND_FREQ)];
    MixChannels(numSamples, leftFrontBuffer, rightFrontBuffer, leftRearBuffer, rightRearBuffer, mixBuffer, flipStereo);

    // Lock SDL audio callback so that it doesn't interfere with following code
    SDL_LockAudio();

    // Calculate number of bytes for current sound chunk
    UINT32 numBytes = numSamples * bytes_per_sample_host;

    // Get end of current play region (writing must occur past this point)
    UINT32 playEndPos = playPos + bytes_per_frame_host;

    // Undo any wrap-around of the write position that may have occured to create following ordering: playPos < playEndPos < writePos
    if (playEndPos > writePos && writeWrapped)
        writePos += audioBufferSize;

    // Check if play region has caught up with write position and now overlaps it (ie buffer under-run)
    if (playEndPos > writePos) {
        underRuns++;

        //printf("Audio buffer under-run #%u in OutputAudio(%u) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u, numBytes = %u]\n",
        //	underRuns, numSamples, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize, numBytes);

        // See what action to take on under-run
        if (underRunLoop) {
            // If loop, then move play position back to beginning of data in buffer
            playPos = writePos + numBytes + bytes_per_frame_host;

            // Check if play position has moved past end of buffer
            if (playPos >= audioBufferSize)
                // If so, wrap it around to beginning again (but keep write wrapped flag as before)
                playPos -= audioBufferSize;
            else {
                // Otherwise, set write wrapped flag as will now appear as if write has wrapped but play position has not
                writeWrapped = true;
                writePos += audioBufferSize;
            }
        } else {
            // Otherwise, bump write position forward in chunks until it is past end of play region
            do {
                writePos += numBytes;
            } while (playEndPos > writePos);
        }
    }

    // Check if write position has caught up with play region and now overlaps it (ie buffer over-run)
    bool overRun = writePos + numBytes > playPos + audioBufferSize;

    bool bufferFull = writePos + 2 * bytes_per_frame_host > playPos + audioBufferSize;

    // Move write position back to within buffer
    if (writePos >= audioBufferSize)
        writePos -= audioBufferSize;

    // Handle buffer over-run
    if (overRun) {
        overRuns++;

        //printf("Audio buffer over-run #%u in OutputAudio(%u) [writePos = %u, writeWrapped = %s, playPos = %u, audioBufferSize = %u, numBytes = %u]\n",
        //	overRuns, numSamples, writePos, (writeWrapped ? "true" : "false"), playPos, audioBufferSize, numBytes);

        bufferFull = true;

        // Discard current chunk of data
        goto Finish;
    }

    src = mixBuffer;
    INT8* dst1;
    INT8* dst2;
    UINT32 len1;
    UINT32 len2;

    // Check if write region extends past end of buffer
    if (writePos + numBytes > audioBufferSize) {
        // If so, split write region into two
        dst1 = audioBuffer + writePos;
        dst2 = audioBuffer;
        len1 = audioBufferSize - writePos;
        len2 = numBytes - len1;
    } else {
        // Otherwise, just copy whole region
        dst1 = audioBuffer + writePos;
        dst2 = 0;
        len1 = numBytes;
        len2 = 0;
    }

    // Copy chunk to write position in buffer
    bytesRemaining = numBytes;
    bytesToCopy = (bytesRemaining > len1 ? len1 : bytesRemaining);
    memcpy(dst1, src, bytesToCopy);

    // Adjust for number of bytes copied
    bytesRemaining -= bytesToCopy;
    src = (INT16*)((UINT8*)src + bytesToCopy);

    if (bytesRemaining) {
        // If write region was split into two, copy second half of chunk into buffer as well
        bytesToCopy = (bytesRemaining > len2 ? len2 : bytesRemaining);
        memcpy(dst2, src, bytesToCopy);
    }

    // Move write position forward for next time
    writePos += numBytes;

    // Check if write position has moved past end of buffer
    if (writePos >= audioBufferSize) {
        // If so, wrap it around to beginning again and set write wrapped flag
        writePos -= audioBufferSize;
        writeWrapped = true;
    }

Finish:
    // Unlock SDL audio callback
    SDL_UnlockAudio();

    // Return whether buffer is half full
    return bufferFull;
}

void CloseAudio()
{
    // Close SDL audio output
    SDL_CloseAudio();

    // Delete audio buffer
    if (audioBuffer != NULL) {
        delete[] audioBuffer;
        audioBuffer = NULL;
    }
}