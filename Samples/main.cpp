#include "Samples/ISampleApp.h"

int main() {
    // ... 플랫폼/엔진 초기화 ...
    std::vector<std::unique_ptr<SampleApp>> samples;
    //samples.push_back(std::make_unique<HelloWorldSample>());
    //samples.push_back(std::make_unique<CubeSample>());
    //samples.push_back(std::make_unique<BatchingSample>());
    // ... 샘플 선택 로직(UI/입력 등) ...
    SampleApp* selectedSample = samples[0].get();

    selectedSample->Initialize(...);

    while (running) {
        selectedSample->Update(dt);
        selectedSample->Render();
    }

    selectedSample->Shutdown();
}