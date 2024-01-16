# SmartTextileGlove: Capturing Complex Hand Movements and Object Interactions Using Machine Learning Powered Stretchable Smart Textile Gloves

Published in Nature Machine Intelligence, 2024 [[Link]](https://www.nature.com/articles/s42256-023-00780-9)

## System requirements
- Graphical user interface: Microsoft Visual Studio C# 2017
- Device firmware: Segger 5.34
- Xcode 14.1
- Unity 2021.2.10.f1
- Python >= 3.9 (package dependencies can be found in Codes/Python/requirements.txt)
## Repository overview
- Codes/
  - iOS software developed using Swift (iOS/)
  - Data acquisition software developed in C# (Data grapher/)
  - Unity demo software (Unity/)
  - Python codes (Python/)
  - Firmware codes developed using C (Firmware/)
  - Data downloader software developed using python (Data receiver/)
- Dataset/
  - Project page: https://feel.ece.ubc.ca/SmartTextileGlove/
    - Dataset collected from five subjects for different applications (Raw data/)
    - Power consumption data for costume-made data acquisition board (PCB Power consumption/)
    - Source data from sensor characteristis (Sensor characteristics/)
    - Output data for click detection (Click detection/)

## Bibtex
If you find this code useful in your research, please cite:

```
@article{tashakori2024capturing,
  title={Capturing complex hand movements and object interactions using machine learning-powered stretchable smart textile gloves},
  author={Tashakori, Arvin and Jiang, Zenan and Servati, Amir and Soltanian, Saeid and Narayana, Harishkumar and Le, Katherine and Nakayama, Caroline and Yang, Chieh-ling and Wang, Z Jane and Eng, Janice J and others},
  journal={Nature Machine Intelligence},
  pages={1--13},
  year={2024},
  publisher={Nature Publishing Group UK London}
}
```
