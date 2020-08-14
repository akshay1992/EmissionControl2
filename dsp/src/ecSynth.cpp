/**
 * ecSynth.cpp
 *
 * AUTHOR: Jack Kilgore
 */

/**** Emission Control LIB ****/
#include "ecSynth.h"
#include "utility.h"

/**** ALLOLIB ****/
#include "al/io/al_File.hpp"

using namespace al;

/**** ecSynth Implementation ****/

void ecSynth::setIO(al::AudioIOData *io) {
  mPrevSR = mGlobalSamplingRate;
  grainScheduler.setSamplingRate(io->fps());
  mGlobalSamplingRate = io->fps();
}

void ecSynth::init(al::AudioIOData *io) {
  int index;

  for (index = 0; index < NUM_MODULATORS; index++) {
    Modulators.push_back(std::make_shared<ecModulator>());
  }

  for (index = 0; index < NUM_MODULATORS; index++) {
    LFOparameters.push_back(new LFOstruct{index});
  }

  mGlobalSamplingRate = io->fps();
  mPrevSR = io->fps();

  mPActiveVoices = &mActiveVoices;

  mScanner.setSamplingRate(mGlobalSamplingRate);

  // MUST USE THIS ORDER
  std::vector<std::string> lfo_names{"LFO1", "LFO2", "LFO3", "LFO4"};

  ECParameters[consts::GRAIN_RATE]->setModulationSource(Modulators[0]);

  ECModParameters[consts::GRAIN_RATE]->setMenuElements(lfo_names);
  ECModParameters[consts::GRAIN_RATE]->registerMenuChangeCallback(
      [&](int value) { grainRate.setModulationSource(Modulators[value]); });

  // grainRateLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  // grainRate.setModulationSource(Modulators[0]); // Default
  // grainRateLFO.registerChangeCallback(
  //     [&](int value) { grainRate.setModulationSource(Modulators[value]); });
  asyncLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  asynchronicity.setModulationSource(Modulators[0]);
  asyncLFO.registerChangeCallback([&](int value) {
    asynchronicity.setModulationSource(Modulators[value]);
  });
  intermittencyLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  intermittency.setModulationSource(Modulators[0]);
  intermittencyLFO.registerChangeCallback(
      [&](int value) { intermittency.setModulationSource(Modulators[value]); });
  streamsLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  streams.setModulationSource(Modulators[0]);
  streamsLFO.registerChangeCallback(
      [&](int value) { streams.setModulationSource(Modulators[value]); });
  grainDurationLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  grainDurationMs.setModulationSource(Modulators[0]);
  grainDurationLFO.registerChangeCallback([&](int value) {
    grainDurationMs.setModulationSource(Modulators[value]);
  });
  envelopeLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  envelope.setModulationSource(Modulators[0]);
  envelopeLFO.registerChangeCallback(
      [&](int value) { envelope.setModulationSource(Modulators[value]); });
  tapeHeadLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  tapeHead.setModulationSource(Modulators[0]);
  tapeHeadLFO.registerChangeCallback(
      [&](int value) { tapeHead.setModulationSource(Modulators[value]); });
  scanSpeedLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  scanSpeed.setModulationSource(Modulators[0]);
  scanSpeedLFO.registerChangeCallback(
      [&](int value) { scanSpeed.setModulationSource(Modulators[value]); });
  scanWidthLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  scanWidth.setModulationSource(Modulators[0]);
  scanWidthLFO.registerChangeCallback(
      [&](int value) { scanWidth.setModulationSource(Modulators[value]); });
  transpositionLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  transposition.setModulationSource(Modulators[0]);
  transpositionLFO.registerChangeCallback(
      [&](int value) { transposition.setModulationSource(Modulators[value]); });

  filterLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  filter.setModulationSource(Modulators[0]);
  filterLFO.registerChangeCallback(
      [&](int value) { filter.setModulationSource(Modulators[value]); });

  resonanceLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  resonance.setModulationSource(Modulators[0]);
  resonanceLFO.registerChangeCallback(
      [&](int value) { resonance.setModulationSource(Modulators[value]); });

  volumeLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  volumeDB.setModulationSource(Modulators[0]);
  volumeLFO.registerChangeCallback(
      [&](int value) { volumeDB.setModulationSource(Modulators[value]); });
  panLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  pan.setModulationSource(Modulators[0]);
  panLFO.registerChangeCallback(
      [&](int value) { pan.setModulationSource(Modulators[value]); });
  soundFileLFO.setElements({"LFO1", "LFO2", "LFO3", "LFO4"});
  soundFile.setModulationSource(Modulators[0]);
  soundFileLFO.registerChangeCallback(
      [&](int value) { soundFile.setModulationSource(Modulators[value]); });

  grainScheduler.configure(ECParameters[consts::GRAIN_RATE]->getParam(), 0.0,
                           0.0);

  // FOR LOOP CAUSES CRASHES ???
  LFOparameters[0]->shape->registerChangeCallback(
      [&](int value) { Modulators[0]->setWaveform(value); });

  LFOparameters[0]->polarity->registerChangeCallback(
      [&](int value) { Modulators[0]->setPolarity(value); });

  LFOparameters[0]->frequency->mParameter->registerChangeCallback(
      [&](float value) { Modulators[0]->setFrequency(value); });

  LFOparameters[0]->duty->registerChangeCallback(
      [&](float value) { Modulators[0]->setWidth(value); });

  LFOparameters[1]->shape->registerChangeCallback(
      [&](int value) { Modulators[1]->setWaveform(value); });

  LFOparameters[1]->polarity->registerChangeCallback(
      [&](int value) { Modulators[1]->setPolarity(value); });

  LFOparameters[1]->frequency->mParameter->registerChangeCallback(
      [&](float value) { Modulators[1]->setFrequency(value); });

  LFOparameters[1]->duty->registerChangeCallback(
      [&](float value) { Modulators[1]->setWidth(value); });

  LFOparameters[2]->shape->registerChangeCallback(
      [&](int value) { Modulators[2]->setWaveform(value); });

  LFOparameters[2]->polarity->registerChangeCallback(
      [&](int value) { Modulators[2]->setPolarity(value); });

  LFOparameters[2]->frequency->mParameter->registerChangeCallback(
      [&](float value) { Modulators[2]->setFrequency(value); });

  LFOparameters[2]->duty->registerChangeCallback(
      [&](float value) { Modulators[2]->setWidth(value); });

  LFOparameters[3]->shape->registerChangeCallback(
      [&](int value) { Modulators[3]->setWaveform(value); });

  LFOparameters[3]->polarity->registerChangeCallback(
      [&](int value) { Modulators[3]->setPolarity(value); });

  LFOparameters[3]->frequency->mParameter->registerChangeCallback(
      [&](float value) { Modulators[3]->setFrequency(value); });

  LFOparameters[3]->duty->registerChangeCallback(
      [&](float value) { Modulators[3]->setWidth(value); });

  /**
   * WHY DOES THIS CRASH ??
   */
  // for (int index = 0; index < NUM_MODULATORS; ++index) {
  //   std::cout << "INDEX: " << index << std::endl;
  //   LFOparameters[index]->shape->registerChangeCallback(
  //       [&](int value) { Modulators[index]->setWaveform(value); });

  //   LFOparameters[index]->frequency->registerChangeCallback(
  //       [&](float value) { Modulators[index]->setFrequency(value); });

  //   LFOparameters[index]->duty->registerChangeCallback(
  //       [&](float value) { Modulators[index]->setWidth(value); });
  // }

  grainSynth.allocatePolyphony<Grain>(2048);
  grainSynth.setDefaultUserData(this);

  /**
   * Input correct number of files into parameters.
   */
  soundFile.mParameter->max(mClipNum);
  soundFile.mLowRange->max(mClipNum);
  soundFile.mHighRange->max(mClipNum);
  soundFile.mHighRange->set(mClipNum);
}

void ecSynth::onProcess(AudioIOData &io) {
  //        updateFromParameters();
  /* Manipulate on a Grain Level */

  while (io()) {
    for (int index = 0; index < NUM_MODULATORS; ++index)
      Modulators[index]->sampleAndStore();

    // THIS IS WHERE WE WILL MODULATE THE GRAIN SCHEDULER

    // NOTE grainRate noise isnt very perceptible
    grainScheduler.setFrequency(ECParameters[consts::GRAIN_RATE]->getModParam(
        ECModParameters[consts::GRAIN_RATE]->getWidthParam()));

    if (modAsynchronicityDepth.getParam() > 0) // modulate the asynchronicity
      grainScheduler.setAsynchronicity(
          asynchronicity.getModParam(modAsynchronicityDepth.getParam()));
    else
      grainScheduler.setAsynchronicity(asynchronicity.getParam());

    if (modIntermittencyDepth.getParam() > 0) // modulate the intermittency
      grainScheduler.setIntermittence(
          intermittency.getModParam(modIntermittencyDepth.getParam()));
    else
      grainScheduler.setIntermittence(intermittency.getParam());

    if (modStreamsDepth.getParam() >
        0) // Modulate the amount of streams playing.
      grainScheduler.setPolyStream(
          consts::synchronous, streams.getModParam(modStreamsDepth.getParam()));
    else
      grainScheduler.setPolyStream(consts::synchronous, streams.getParam());

    mCurrentIndex = mScanner();
    // CONTROL RATE LOOP (Executes every 4th sample)
    if (controlRateCounter == 4) {
      controlRateCounter = 0;
      mPrevModClip = mModClip;
      mModClip = soundFile.getModParam(modSoundFileDepth.getParam()) - 1;
    }
    controlRateCounter++;
    /////

    // Grain by Grain Initilization
    if (grainScheduler.trigger()) {
      prevTapeHeadVal = nowTapeHeadVal;
      nowTapeHeadVal = tapeHead.getModParam(modTapeHeadDepth.getParam());
      prev_scan_speed = scan_speed;
      scan_speed = scanSpeed.getModParam(modScanSpeedDepth.getParam());
      prev_scan_width = scan_width;
      scan_width = scanWidth.getModParam(modScanWidthDepth.getParam());
      float frames = soundClip[mModClip]->frames;
      float start, end;

      /**ODD BUG: when scan speed is higher than 1.015, the volume is gut in
       * halfwut**/

      // Reset index.
      if (mPrevModClip != mModClip || mCurrentIndex == mScanner.getTarget() ||
          prevTapeHeadVal != nowTapeHeadVal) {
        start = nowTapeHeadVal * frames;
        if (scan_speed >= 0) {
          end = start + ((frames - start) * scan_width);
          mScanner.set(start, end,
                       (end - start) / (mGlobalSamplingRate * scan_speed));
        } else {
          end = start * (1 - scan_width);
          mScanner.set(start, end,
                       (start - end) / (mGlobalSamplingRate * abs(scan_speed)));
        }
      }
      // On the fly adjustments.
      if (scan_width != prev_scan_width || scan_speed != prev_scan_speed) {
        start = mScanner.getValue();
        if (scan_speed >= 0) {
          end = start + ((frames - start) * scan_width);
          mScanner.set(start, end,
                       (end - start) / (mGlobalSamplingRate * scan_speed));
        } else {
          end = start * (1 - scan_width);
          mScanner.set(start, end,
                       (start - end) / (mGlobalSamplingRate * abs(scan_speed)));
        }
      }

      // if (mCurrentIndex == mScanner.getTarget()) {
      //   if (scan_speed >= 0)
      //     mScanner.set(0, frames,
      //                  (frames) / (mGlobalSamplingRate * scan_speed));
      //   else
      //     mScanner.set(frames, 0,
      //                  ((frames)) / (mGlobalSamplingRate * abs(scan_speed)));
      // }

      /* Experiments

      if (prevTapeHeadVal != nowTapeHeadVal) {
        start = mScanner.getValue();
        end = (nowTapeHeadVal * frames) +
              ((frames - (nowTapeHeadVal * frames)) * scan_width);
        mScanner.set(start, end,
                     (end - start) / (mGlobalSamplingRate * scan_speed));
      }

      */

      /* Experiment: Quick attack to new tapehead position.
      if (prevTapeHeadVal != nowTapeHeadVal) {
        if (mScanner.getValue() < nowTapeHeadVal * frames) {
          start = mScanner.getValue();
          end = nowTapeHeadVal * frames;
          mScanner.set(start, end,
                       abs(end - start) / (mGlobalSamplingRate * scan_speed) /
                           (scan_speed * 16));
        } else {
          start = nowTapeHeadVal * frames;
          end = mScanner.getValue();
          mScanner.set(start, end,
                       abs(start - end) / (mGlobalSamplingRate * scan_speed) /
                           (scan_speed * 16));
        }
      }
      */

      auto *voice = static_cast<Grain *>(grainSynth.getFreeVoice());
      if (voice) {
        grainParameters list = {
            grainDurationMs,
            modGrainDurationDepth.getParam(),
            envelope,
            modEnvelopeDepth.getParam(),
            tapeHead,
            modTapeHeadDepth.getParam(),
            transposition,
            modTranspositionDepth.getParam(),
            filter,
            modFilterDepth.getParam(),
            resonance,
            modResonanceDepth.getParam(),
            volumeDB,
            modVolumeDepth.getParam(),
            pan,
            modPanDepth.getParam(),
            soundClip[mModClip],
            mPActiveVoices,
            mCurrentIndex,
        };

        voice->configureGrain(list, mGlobalSamplingRate);

        mActiveVoices++;
        grainSynth.triggerOn(voice, io.frame());

      } else {
        std::cout << "out of voices!" << std::endl;
      }
    }
  }

  grainSynth.render(io);

  io.frame(0);
  // Manipulate on a stream level
  while (io()) {
    // Set clip vars to > 0 if clip is detected on any sample
    // 5 is the number of visual frames the meter will turn red for
    if (io.out(0) > 1.0f || io.out(0) < -1.0f)
      clipL = 5;
    if (io.out(1) > 1.0f || io.out(1) < -1.0f)
      clipR = 5;
    // add samples to oscilloscope ringbuffer
    oscBufferL.push_back(io.out(0));
    oscBufferR.push_back(io.out(1));
    // Add samples to VU ringbuffer (squared for RMS calculations)
    vuBufferL.push_back(pow(io.out(0), 2));
    vuBufferR.push_back(pow(io.out(1), 2));
  }
}

void ecSynth::onTriggerOn() {}

void ecSynth::onTriggerOff() {}

void ecSynth::loadSoundFile(std::string fileName) {
  if (std::find(soundClipFileName.begin(), soundClipFileName.end(), fileName) !=
      soundClipFileName.end())
    return;
  bool temp = util::load(fileName, soundClip, mGlobalSamplingRate, true);
  if (temp) {
    soundClipFileName.push_back(fileName);
    mClipNum++;
    soundFile.mParameter->max(mClipNum);
    soundFile.mLowRange->max(mClipNum);
    soundFile.mHighRange->max(mClipNum);
    soundFile.mHighRange->set(mClipNum); // stylistic choice, might take out
  }
}

bool ecSynth::loadInitSoundFiles(std::string directory) {
  FileList initAudioFiles = fileListFromDir(directory);
  initAudioFiles.sort(util::compareFileNoCase);

  bool success = false;
  for (auto i = initAudioFiles.begin(); i != initAudioFiles.end(); i++) {
    if (i->file().substr(i->file().length() - 4) == ".wav" ||
        i->file().substr(i->file().length() - 4) == ".aif") {
      loadSoundFile(i->filepath());
      success = true;
    } else if (i->file().substr(i->file().length() - 5) == ".aiff") {
      loadSoundFile(i->filepath());
      success = true;
    }
  }
  return success;
}

bool ecSynth::removeSoundFile(int index) {
  if (mClipNum == 0)
    return false;
  soundClip.erase(soundClip.begin() + index);
  soundClipFileName.erase(soundClipFileName.begin() + index);
  mClipNum--;
  soundFile.mParameter->max(mClipNum);
  soundFile.mLowRange->max(mClipNum);
  soundFile.mHighRange->max(mClipNum);
  soundFile.mHighRange->set(mClipNum); // stylistic choice, might take out

  if (static_cast<int>(soundFile.mParameter->get()) >= index)
    soundFile.mParameter->set(soundFile.mParameter->get() - 1);
  return true;
}

bool ecSynth::removeCurrentSoundFile() {
  removeSoundFile(soundFile.mParameter->get() - 1);
  return true;
}

void ecSynth::clearSoundFiles() {
  soundClip.clear();
  soundClipFileName.clear();

  mClipNum = 0;
  soundFile.mParameter->max(mClipNum);
  soundFile.mLowRange->max(mClipNum);
  soundFile.mHighRange->max(mClipNum);
  soundFile.mHighRange->set(mClipNum); // stylistic choice, might take out
}

void ecSynth::resampleSoundFiles() {
  // If sampling rate is the same as before, no need for resampling.
  if (static_cast<int>(mPrevSR) == static_cast<int>(mGlobalSamplingRate))
    return;
  std::vector<std::string> filePaths;
  long unsigned i;
  // Collect filepaths of audio buffers.
  for (i = 0; i < soundClip.size(); i++) {
    filePaths.push_back(soundClip[i]->filePath);
  }

  clearSoundFiles();
  for (long unsigned i = 0; i < filePaths.size(); i++)
    loadSoundFile(filePaths[i]);
}

void ecSynth::hardClip(al::AudioIOData &io) {
  for (unsigned i = 0; i < io.channelsOut(); ++i) {
    if (io.out(i) > 1)
      io.sum(-1 * io.out(i) + 1, i);
    if (io.out(i) < -1)
      io.sum(-1 * io.out(i) - 1, i);
  }
}

/**
 * WIP -- runs at max 2/3 of full power
 * softClip(currentSample) :
 *    -2/3    if x < -1
 *     2/3    if x > 1
 *     currentSample - (currentSample)**3/3
 */
void ecSynth::softClip(al::AudioIOData &io) {
  for (unsigned int i = 0; i < io.channelsOut(); ++i) {
    float currentSample = io.out(i);
    if (currentSample > 1)
      io.sum(-1 * currentSample + (2.0f / 3), i);
    else if (currentSample < -1)
      io.sum(-1 * currentSample - (2.0f / 3), i);
    else
      io.sum(-1 * powf(currentSample, 3) / 3, i);
  }
}

/**** TO DO TO DO TO DO ****/
void ecSynth::throttle(float time, float ratio) {
  if (mCounter < time * mGlobalSamplingRate) {
    mCounter++;
    mAvgActiveVoices += mActiveVoices;
    return;
  } else {
    mCounter++;
    mAvgActiveVoices /= mCounter;
    mCounter = 0;
  }

  // float adaptThresh;

  // if (mPeakCPU > adaptThresh) {
  // }
  // if (mAvgCPU > adaptThresh) {
  // } else {
  // }
}