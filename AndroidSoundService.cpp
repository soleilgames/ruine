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

#include "AndroidSoundService.hpp"
#include "stringutils.hpp"

namespace Soleil {

  SLException::SLException(const char* what, SLresult errorCode)
    : std::runtime_error(toString("(", errorCode, ") ", what))
    , errorCode(errorCode)
  {
  }

  SLException::~SLException() {}

  SLresult SLException::getErrorCode(void) const noexcept { return errorCode; }

  AndroidSoundService::AndroidSoundService(AssetService* assetService)
    : assetService(assetService)
  {
    /* Engine Interface */
    engineObj.invoke(slCreateEngine, 0, nullptr, 0, nullptr, nullptr);
    engineObj.realize(Async::False);
    engineObj.getInterface(SL_IID_ENGINE, &(engine.getEngine()));

    SOLEIL__LOGGER_DEBUG("Before realizing SL Engine");

    /* Output Interface */
    engine.createOutputMix(outputMixObj, {});
    outputMixObj.realize(Async::False);

    /* Sounds initialization */
    SLDataLocator_AndroidSimpleBufferQueue bufferLocator = {
      SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM pcmFormat = {
      SL_DATAFORMAT_PCM,           1,
      SL_SAMPLINGRATE_44_1,        SL_PCMSAMPLEFORMAT_FIXED_16,
      SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_CENTER,
      SL_BYTEORDER_LITTLEENDIAN};

    SLDataSource            audioSource      = {&bufferLocator, &pcmFormat};
    SLDataLocator_OutputMix locatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX,
                                                outputMixObj.getObject()};
    SLDataSink audioSink = {&locatorOutputMix, nullptr};

    const InterfaceRequestMap soundRequest = {
      {SL_IID_BUFFERQUEUE, SL_BOOLEAN_TRUE},
      {SL_IID_VOLUME, SL_BOOLEAN_TRUE},
      {SL_IID_EFFECTSEND, SL_BOOLEAN_TRUE},
      {SL_IID_VOLUME, SL_BOOLEAN_TRUE}};

    engine.createAudioPlayer(soundPlayerObj, audioSource, audioSink,
                             soundRequest);
    soundPlayerObj.realize(Async::False);
    soundPlayerObj.getInterface(SL_IID_PLAY, &(soundPlayer.getPlayer()));
    soundPlayerObj.getInterface(SL_IID_BUFFERQUEUE, &(soundBuffer.data()));
    soundPlayerObj.getInterface(SL_IID_EFFECTSEND, &(soundEffectSend));
    soundPlayerObj.getInterface(SL_IID_VOLUME, &soundVolume);

    soundPlayer.setPlayState(PlayState::playing);

    SOLEIL__LOGGER_DEBUG("Creating new SL Engine");
  }

  AndroidSoundService::~AndroidSoundService()
  {
    SOLEIL__LOGGER_DEBUG("SL Engine destructed");
  }

  void AndroidSoundService::playMusic(const std::string& trackName)
  {
    /* Unlikely to sound, we create a Player each time we want to play a new
     * music*/
    AssetDescriptorPtr ad = assetService->asDescriptor(trackName);

    /* Configure Audio source */
    SLDataLocator_AndroidFD locatorFd = {SL_DATALOCATOR_ANDROIDFD, ad->getFd(),
                                         ad->getStart(), ad->getLength()};
    SLDataFormat_MIME formatMime = {SL_DATAFORMAT_MIME, nullptr,
                                    SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSource = {&locatorFd, &formatMime};

    /* Configure Audio sink: */
    SLDataLocator_OutputMix locatorOutputMix = {SL_DATALOCATOR_OUTPUTMIX,
                                                outputMixObj.getObject()};
    SLDataSink audioSink = {&locatorOutputMix, nullptr};

    // -----------

    const InterfaceRequestMap request = {{SL_IID_SEEK, SL_BOOLEAN_TRUE},
                                         {SL_IID_MUTESOLO, SL_BOOLEAN_TRUE},
                                         {SL_IID_VOLUME, SL_BOOLEAN_TRUE}};
    engine.createAudioPlayer(playerObj, audioSource, audioSink, request);

    playerObj.realize(Async::False);

    playerObj.getInterface(SL_IID_PLAY, &(player.getPlayer()));

    playerObj.getInterface(SL_IID_VOLUME, &volume);

    const int level       = 10;
    const int attenuation = 100 - 50;
    const int millibel    = attenuation * -50;
    SLresult  result      = (*volume)->SetVolumeLevel(volume, millibel);

    player.setPlayState(PlayState::playing);
  }

  bool AndroidSoundService::pauseMusic(void)
  {
    if (player.isInitialized() && playerObj.isRealized()) {
      player.setPlayState(PlayState::paused);
      return true;
    }
    return false;
  }

  bool AndroidSoundService::resumeMusic(void)
  {
    if (player.isInitialized() && playerObj.isRealized()) {
      // TODO:  if not playing ?
      player.setPlayState(PlayState::playing);
      return true;
    }
    return false;
  }

  void AndroidSoundService::fireSound(const std::string&     sound,
                                      const SoundProperties& properties)
  {
    assert(soundPlayerObj.isRealized() &&
           "Sound Player was not in realize state");

    if (soundPlayerObj.isRealized()) {
      soundPlayer.setPlayState(PlayState::stopped);

      soundBuffer.clear();
      const auto& buffer = loadSound(sound);
      soundBuffer.enqueue(buffer);

      const int level       = 10;
      const int attenuation = 100 - properties.volume;
      const int millibel    = attenuation * -50;
      SLresult  result = (*soundVolume)->SetVolumeLevel(soundVolume, millibel);
      assert(SL_RESULT_SUCCESS == result &&
             "Android state that this never fails");
      (void)result;
      soundPlayer.setPlayState(PlayState::playing);
    }
  }

  const PcmBuffer& AndroidSoundService::loadSound(const std::string& fileName)
  {
    // TODO: Might be better to allow load and caching sound while loading
    // application. Chaching should be a pure vector to benefit cpu cache?

    auto const found = soundCache.find(fileName);
    if (found != soundCache.end()) {
      return found->second;
    }

    auto buffer = assetService->asDataVector(fileName.c_str());
    return (soundCache.emplace(fileName, buffer).first)->second;
  }
} // Soleil
