#pragma once

#include <memory>

#include <portaudio.h>

#include "logger/logger.h"
#include "core/audio_object/audio_object.h"

namespace hpaslt {

class AudioPlayer {
 private:
  /**
   * @brief Currently playing AudioObject.
   *
   */
  std::shared_ptr<AudioObject> m_audioObj;

  /**
   * @brief Created stream for current AudioObject.
   *
   */
  PaStream *m_stream;

  /**
   * @brief If the player is currently playing.
   *
   */
  bool m_isPlaying;

  /**
   * @brief Callback function called by Port Audio.
   *
   * @param inputBuffer ignore input buffer.
   * @param outputBuffer write to this buffer to play audio.
   * @param framesPerBuffer number of frames to write.
   * @param timeInfo PaStreamCallbackTimeInfo pointer.
   * @param statusFlags PaStreamCallbackFlags struct.
   * @param userData AudioObject pointer as void*.
   * @return paContinue if there's more data to play.
   * @return paComplete if the audio ends.
   */
  static int paCallback(const void *inputBuffer, void *outputBuffer,
                        unsigned long framesPerBuffer,
                        const PaStreamCallbackTimeInfo *timeInfo,
                        PaStreamCallbackFlags statusFlags, void *userData);

 public:
  static void portAudioError(PaError err) {
    logger->coreLogger->error("Port Audio Error: {}", Pa_GetErrorText(err));
  }

  /**
   * @brief Initialize AudioPlayer backend (Port Audio).
   *
   */
  static void initAudioPlayer() {
    // Initialize PortAudio.
    PaError err = Pa_Initialize();
    if (err != paNoError) {
      portAudioError(err);
      return;
    }
    logger->coreLogger->debug("PortAudio initialized.");
  }

  /**
   * @brief Terminate AudioPlayer backend (Port Audio).
   *
   */
  static void terminateAudioPlayer() {
    // Terminate PortAudio.
    PaError err = Pa_Terminate();
    if (err != paNoError) {
      portAudioError(err);
      return;
    }
    logger->coreLogger->debug("PortAudio terminated.");
  }

  /**
   * @brief Construct a new AudioPlayer object.
   *
   */
  AudioPlayer();

  /**
   * @brief Destroy the AudioPlayer object.
   *
   */
  ~AudioPlayer();

  /**
   * @brief Load the AudioObject to the AudioPlayer.
   *
   * @param audioObj
   */
  void loadAudioObject(std::weak_ptr<AudioObject> audioObj);

  /**
   * @brief Play the audio file from current cursor position.
   *
   */
  void play();

  /**
   * @brief Pause the audio.
   *
   */
  void pause();

  /**
   * @brief Reset the cursor to 0 and play.
   *
   */
  void replay();

  /**
   * @brief Stop and reset the cursor to 0.
   *
   */
  void stop();
};

}  // namespace hpaslt
