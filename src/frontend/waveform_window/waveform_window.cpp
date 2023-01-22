#include <imgui.h>
#include <implot.h>

#include "core/audio_workspace/audio_workspace.h"
#include "waveform_window.h"

#define AUDIO_WAVEFORM_RESOLUTION 8192

namespace hpaslt {

eventpp::CallbackList<void(bool)> WaveformWindow::s_onEnable;

void WaveformWindow::downSampleLayer(AudioDataLayer& layer) {
  std::shared_ptr<std::vector<float>> wx = layer.wx;
  std::shared_ptr<std::vector<float>> wy = layer.wy;

  layer.sampleSize /= 2;
  layer.sampleRate /= 2;
  layer.wx = std::make_shared<std::vector<float>>(layer.sampleSize);
  layer.wy = std::make_shared<std::vector<float>>(layer.sampleSize);

  for (int i = 0; i < wx->size(); i++) {
    // Keep odd index.
    if (i % 2) {
      (*layer.wx)[i / 2] = (*wx)[i];
      (*layer.wy)[i / 2] = (*wy)[i];
    }
  }
}

WaveformWindow::WaveformWindow()
    : ImGuiObject("Waveform"),
      m_channelNum(0),
      m_sampleRate(0),
      m_sampleSize(0) {
  // Setup window enable callback.
  setupEnableCallback(s_onEnable);

  // Setuo audio loaded callback.
  m_audioLoadedHandle = AudioWorkspace::s_onAudioLoaded.append(
      [&](std::weak_ptr<AudioObject> audioObj) {
        logger->coreLogger->trace("WaveformWindow load new AudioObject.");

        // Load m_audioObj.
        m_audioMutex.lock();

        m_audioObj = audioObj.lock();

        // Copy all the audio data
        m_channelNum = m_audioObj->getAudioFile().getNumChannels();
        m_sampleRate = m_audioObj->getAudioFile().getSampleRate();
        m_sampleSize = m_audioObj->getAudioFile().getNumSamplesPerChannel();
        auto& samples = m_audioObj->getAudioFile().samples;
        logger->coreLogger->trace("WaveformWindow copied audio data.");

        m_audioChannels.clear();
        AudioChannel audioChannel;
        for (int channel = 0; channel < m_channelNum; channel++) {
          // Push full resolution layer.
          AudioDataLayer audioLayer;
          audioLayer.wx = std::make_shared<std::vector<float>>(m_sampleSize);
          audioLayer.wy = std::make_shared<std::vector<float>>(m_sampleSize);
          audioLayer.sampleSize = m_sampleSize;
          audioLayer.sampleRate = m_sampleRate;
          for (int i = 0; i < audioLayer.sampleSize; i++) {
            (*audioLayer.wx)[i] = (float)i / (float)m_sampleRate;
            (*audioLayer.wy)[i] = samples[channel][i];
          }
          audioChannel.layers.push_back(audioLayer);

          // Down sample layers.
          while (audioLayer.sampleSize > AUDIO_WAVEFORM_RESOLUTION) {
            downSampleLayer(audioLayer);
            audioChannel.layers.push_back(audioLayer);
          }

          m_audioChannels.push_back(audioChannel);
        }
        logger->coreLogger->trace("WaveformWindow generated {} channel data.",
                                  m_channelNum);

        m_audioMutex.unlock();
      });
}

WaveformWindow::~WaveformWindow() {
  // Reset window enable callback.
  resetEnableCallback(s_onEnable);
  // Remove audio loaded callback.
  AudioWorkspace::s_onAudioLoaded.remove(m_audioLoadedHandle);
}

void WaveformWindow::render() {
  // Init window properties.
  const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
  ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
  ImGui::Begin("Waveform", nullptr, windowFlags);

  if (m_audioMutex.try_lock()) {
    if (m_channelNum > 0 &&
        ImPlot::BeginSubplots("Audio Channels", m_channelNum, 1, ImVec2(-1, -1),
                              ImPlotSubplotFlags_LinkAllX)) {
      for (int channel = 0; channel < m_channelNum; channel++) {
        AudioChannel& audioChannel = m_audioChannels[channel];
        std::stringstream channelName;
        channelName << "Channel " << channel;
        if (ImPlot::BeginPlot(channelName.str().c_str())) {
          ImPlot::SetupAxes("Time", "Amplitude");
          ImPlot::SetupAxisLimits(ImAxis_Y1, -1, 1, ImPlotCond_Always);
          auto widthPix = ImPlot::GetPlotSize().x;
          int start = ImPlot::GetPlotLimits().X.Min * m_sampleRate;
          int end = ImPlot::GetPlotLimits().X.Max * m_sampleRate;
          // Clamp start and end.
          start = start < 0 ? 0 : start;
          start = start > m_sampleSize ? m_sampleSize : start;
          end = end < 0 ? 0 : end;
          end = end > m_sampleSize ? m_sampleSize : end;
          // Resample data.
          int span = end - start;
          // Find best layer.
          int i = audioChannel.layers.size() - 1;
          while (i > 0 &&
                 (span * audioChannel.layers[i].sampleRate / m_sampleRate <
                  AUDIO_WAVEFORM_RESOLUTION)) {
            i--;
          }
          AudioDataLayer layer = audioChannel.layers[i];
          int layerStart =
              start * audioChannel.layers[i].sampleRate / m_sampleRate;
          int layerSpan =
              span * audioChannel.layers[i].sampleRate / m_sampleRate;

          ImPlot::PlotLine("", layer.wx->data() + layerStart,
                           layer.wy->data() + layerStart, layerSpan);
          ImPlot::EndPlot();
        }
      }

      ImPlot::EndSubplots();
    }
    m_audioMutex.unlock();
  }

  ImGui::End();
}

}  // namespace hpaslt