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

  AndroidSoundService::AndroidSoundService()
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
      SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 20};
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
      {SL_IID_EFFECTSEND, SL_BOOLEAN_TRUE}};

    engine.createAudioPlayer(soundPlayerObj, audioSource, audioSink,
                             soundRequest);
    soundPlayerObj.realize(Async::False);
    soundPlayerObj.getInterface(SL_IID_PLAY, &(soundPlayer.getPlayer()));
    // TODO: get the volume interface
    soundPlayerObj.getInterface(SL_IID_BUFFERQUEUE, &(soundBuffer.data()));
    soundPlayerObj.getInterface(SL_IID_EFFECTSEND, &(soundEffectSend));

    soundPlayer.setPlayState(PlayState::playing);

    SOLEIL__LOGGER_DEBUG("Creating new SL Engine");
  }

  AndroidSoundService::~AndroidSoundService()
  {
    SOLEIL__LOGGER_DEBUG("SL Engine destructed");
  }

  void AndroidSoundService::playMusic(const std::string& trackName) {}
  void AndroidSoundService::stopMusic(void) {}
  void AndroidSoundService::fireSound(const std::string& sound) {}

} // Soleil
