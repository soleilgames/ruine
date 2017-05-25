/*
 * Copyright (C) 2017  Florian GOLESTIN
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SOLEIL__ANDROIDSOUNDSERVICE_HPP_
#define SOLEIL__ANDROIDSOUNDSERVICE_HPP_

#include "Logger.hpp"
#include "SoundService.hpp"

#include <map>
#include <pthread.h>
#include <vector>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace Soleil {

  // UTILS ---------------------------------
  enum Async
  {
    True  = SL_BOOLEAN_TRUE,
    False = SL_BOOLEAN_FALSE
  };

  /**
   * To be thrown if OpenSL fails
   */
  class SLException : public std::runtime_error
  {
  public:
    SLException(const char* what, SLresult errorCode);
    virtual ~SLException();
    SLresult getErrorCode(void) const noexcept;

  private:
    SLresult errorCode;
  };

  typedef std::map<SLInterfaceID, SLboolean> InterfaceRequestMap;

  /**
   * Forward the call to the OpenSL Method and throw an exception if it failed
   */
  template <typename F, typename... Args> auto slCheck(F&& f, Args&&... args)
  {
    SLresult result = std::forward<F>(f)(std::forward<Args>(args)...);
    if (result != SL_RESULT_SUCCESS) {
      throw SLException("SL call failed", result);
    }
  }

  /**
 * Wrapper for SLObjectItf
 */
  class SLObject
  {
  public:
    SLObject()
      : object(nullptr)
    {
    }

    virtual ~SLObject()
    {
      if (object != nullptr) {
        (*object)->Destroy(object);
        object = nullptr;
      }
    }

    template <typename F, typename... Args> auto invoke(F&& f, Args&&... args)
    {
      SLresult result =
        std::forward<F>(f)(&object, std::forward<Args>(args)...);
      if (result != SL_RESULT_SUCCESS) {
        throw SLException("SL call failed", result);
      }
    }

    template <typename F, typename... Args> auto self(F&& f, Args&&... args)
    {
      SLresult result = std::forward<F>(f)(object, std::forward<Args>(args)...);
      if (result != SL_RESULT_SUCCESS) {
        throw SLException("SL call failed", result);
      }
    }

    inline void realize(Async async)
    {
      checkInitialized();

      this->self((*object)->Realize, async);
    }

    bool isRealized(void)
    {
      SLuint32 state;

      if (object != nullptr) {
        this->self((*object)->GetState, &state);
        if (state == SL_OBJECT_STATE_REALIZED) {
          return true;
        }
      }
      return false;
    }

    inline void getInterface(SLInterfaceID iid, void* interface)
    {
      checkInitialized();

      this->self((*object)->GetInterface, iid, interface);
    }

    inline SLObjectItf& getObject(void) { return object; }

  protected:
    inline void checkInitialized(void)
    {
      if (object == nullptr) {
        throw SLException("Object was not initialized yet", -42);
      }
    }

  private:
    SLObjectItf object;
  };

  /**
   * Wraper for SLEngineItf
   */
  class SLEngine
  {
  public:
    SLEngine()
      : engine(nullptr)
    {
    }

    virtual ~SLEngine() { engine = nullptr; }

    SLEngineItf& getEngine(void) { return engine; }

    void createOutputMix(SLObject& object, const InterfaceRequestMap& request)
    {
      checkInitialized();

      std::vector<SLInterfaceID> keys;
      std::vector<SLboolean>     values;
      SLuint32                   size = 0;

      for (auto&& item : request) {
        keys.push_back(item.first);
        values.push_back(item.second);
        size++;
      }
      slCheck((*engine)->CreateOutputMix, engine, &(object.getObject()), size,
              keys.data(), values.data());
    }

    void createAudioPlayer(SLObject& object, SLDataSource& audioSource,
                           SLDataSink&                audioSink,
                           const InterfaceRequestMap& request)
    {
      checkInitialized();

      // TODO: same code as createOuputMix
      std::vector<SLInterfaceID> keys;
      std::vector<SLboolean>     values;
      SLuint32                   size = 0;

      for (auto&& item : request) {
        keys.push_back(item.first);
        values.push_back(item.second);
        size++;
      }

      slCheck((*engine)->CreateAudioPlayer, engine, &(object.getObject()),
              &audioSource, &audioSink, size, keys.data(), values.data());
    }

  protected:
    inline void checkInitialized(void)
    {
      if (engine == nullptr) {
        throw SLException("Engine was not initialized yet", -42);
      }
    }

  private:
    SLEngineItf engine;
  };

  enum PlayState
  {
    stopped = SL_PLAYSTATE_STOPPED,
    paused  = SL_PLAYSTATE_PAUSED,
    playing = SL_PLAYSTATE_PLAYING
  };

  /**
   * Wrapper for SLPlayItf
   */
  class SLPlayer
  {
  public:
    SLPlayer()
      : player(nullptr)
    {
    }

    virtual ~SLPlayer() {}

    SLPlayItf& getPlayer(void) { return player; }

    void setPlayState(const PlayState state)
    {
      checkInitialized();

      slCheck((*player)->SetPlayState, player, state);
    }

    bool isInitialized(void) const { return player != nullptr; }

  protected:
    inline void checkInitialized(void)
    {
      if (isInitialized() == false) {
        throw SLException("Player was not initialized yet", -42);
      }
    }

  private:
    SLPlayItf player;
  };

  typedef std::vector<uint8_t> PcmBuffer;

  class SLBufferQueue
  {
  public:
    SLBufferQueue() {}

    virtual ~SLBufferQueue() {}

    void clear(void) { slCheck((*bufferQueue)->Clear, bufferQueue); }

    void enqueue(const PcmBuffer& buffer)
    {
      SOLEIL__LOGGER_DEBUG("Data to bufferize: ", buffer.size());
      slCheck((*bufferQueue)->Enqueue, bufferQueue, (uint8_t*)buffer.data(),
              buffer.size());
    }

    SLBufferQueueItf& data(void) { return bufferQueue; }

  protected:
    inline void checkInitialized(void)
    {
      if (bufferQueue == nullptr) {
        throw SLException("Buffer Queue was not initialized yet", -42);
      }
    }

  private:
    SLBufferQueueItf bufferQueue;
  };

  class AndroidSoundService : public SoundService
  {
  public:
    AndroidSoundService();
    virtual ~AndroidSoundService();

  public:
    void playMusic(const std::string& trackName) override;
    void stopMusic(void) override;
    void fireSound(const std::string& sound) override;

  private:
    SLObject engineObj;
    SLEngine engine;
    SLObject outputMixObj;

  private: // Can be a Player Class
    SLObject    playerObj;
    SLPlayer    player;
    SLVolumeItf volume; // aka Music Volume

  private: // main Sound queue
    SLObject        soundPlayerObj;
    SLPlayer        soundPlayer;
    SLBufferQueue   soundBuffer;
    SLEffectSendItf soundEffectSend;
    std::map<std::string, PcmBuffer> soundCache;
  };

} // Soleil

#endif /* SOLEIL__ANDROIDSOUNDSERVICE_HPP_ */
