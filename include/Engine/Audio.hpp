//
// Created by cervi on 23/08/2022.
//

#ifndef UNDERTALE_AUDIO_HPP
#define UNDERTALE_AUDIO_HPP

#include <cstdio>
#include <cstring>
#include <list>
#include <memory>
#include <nds.h>
#include <string>
#include <utility>

namespace Audio2 {
class AudioManager;

constexpr int kAudioBuffer = 0x800; // in samples, better if it's a power of 2

class AudioFile {
public:
  /**
   * Loads the audio file corresponding to the name passed as a parameter.
   * @param name The name of the audio file to load.
   */
  virtual void load(const std::string &name) = 0;

  /**
   * Gets the filename that was used when load was called.
   * @return The filename used when load was called, or the empty string if load
   * was never called.
   */
  std::string getFilename() { return _filename; }

  /**
   * Sets the number of times to loop the sample.
   * If you call it with loops = 1, the sample will play twice (looping once).
   * If you set loops to -1, the sample will loop indefinitely.
   * @param loops The number of times to loop the sample.
   */
  void setLoops(int loops) { _loops = loops; }

  /**
   * Get whether the file is a stereo audio file.
   * @return Whether the file is stereo.
   */
  bool getStereo() const { return _stereo; }

  /**
   * Get whether the file is playing at the moment.
   * @return Whether the file is playing.
   */
  bool getPlaying() const { return _active; }

  /**
   * Get whether the file has been loaded.
   * @return Whether the file has been loaded.
   */
  bool getLoaded() const { return _loaded; }

  /**
   * Get the current volume of the audio file.
   * @return The volume of the audio (from min=0 to max=127).
   */
  u8 getVolume() const { return _volume; }

  /**
   * Set the current volume of the audio file.
   * @param volume The volume of the audio (from min=0 to max=127).
   */
  void setVolume(u8 volume);

  virtual ~AudioFile() {
    nocashMessage("Destroying AudioFile!");
    if (_active)
      stop();
  }

protected:
  /**
   * Allocates the audio buffers required to play the sample. Should be called
   * after loading the audio file. Frees any existing audio buffers. Used the
   * internal _bitsPerSample and _stereo values to allocate kAudioBuffer samples
   * to the needed audio buffers, so these two values should be set before
   * calling this function.
   */
  void allocateBuffers();

  /**
   * Plays the audio file. If it is already playing, it is first stopped and
   * then played again. Progresses the first half of the audio buffer.
   */
  bool play();

  /**
   * Stops the audio file if it is playing. If the file has been setup as
   * self-freeing, then it frees itself.
   */
  void stop();

  /**
   * Should reset the playing audio file to the beginning of the playback,
   * resetting any needed internal values.
   */
  virtual void resetPlaying() = 0;

  int _leftChannel = -1;
  // When allocated has kAudioBuffer samples of size
  // The size of each sample is determined by _bitsPerSample.
  std::unique_ptr<u8[]> _leftBuffer = nullptr;

  int _rightChannel = -1;
  // When allocated has kAudioBuffer samples of size
  // The size of each sample is determined by _bitsPerSample.
  std::unique_ptr<u8[]> _rightBuffer = nullptr;

  // The position in the audio buffers in samples.
  u32 _sampleBufferPos;

  virtual SoundFormat getAllocFormat() = 0;

  virtual u8 getBitsPerSample() = 0;
  /**
   * Should only be called from update(). Should stream the desired samples
   * to the audio buffers.
   * @param samples The number of samples to load.
   */
  virtual void progress(u16 samples) = 0;

  std::string _filename;
  bool _active = false;
  bool _stereo = false;
  bool _loaded = false;
  int _loops = 0; // TODO: Maybe copy to internal value when played?
  u32 _expectedSampleBufferPos;
  u16 _sampleRate = 1;
  u8 _volume = 127;

private:
  /**
   * Should be called in every frame to stream the audio to the buffers.
   * Is called by the AudioManager.
   */
  void update();
  u16 _timerLast;
  u32 _ticksRemain;

  friend class AudioManager;
};

class AudioManager {
public:
  /**
   * Initialize the audio system. Instruct the audio system to use
   * timerId as the timer for timing audio sample loading.
   * @param timerId The id of the timer to use.
   */
  explicit AudioManager(int timerId);

  void play(std::shared_ptr<AudioFile> audio_file);
  void stop(const std::shared_ptr<AudioFile> &audio_file);

  /**
   * Should be called in every frame. Called from Engine::tick, and iterates
   * through all the currently playing audio files and updates them.
   */
  void update();

  /**
   * Gets the timer used by the audio system, specified when the audio manager
   * is constructed.
   * @return The timer used by the audio system.
   */
  u8 getTimerId() const { return _timerId; }

private:
  std::list<std::shared_ptr<AudioFile>> _playing;
  u8 _timerId;
};

extern AudioManager audioManager;
} // namespace Audio2

#endif // UNDERTALE_AUDIO_HPP
