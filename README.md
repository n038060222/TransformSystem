#Matrix Transformer System
Overview
This project implements a Matrix Transformer system consisting of three separate applications: appX, appY, and appUI. These applications work together to generate, transform, and display matrix data in real-time.

Applications
ApplicationX (appX)
Description:

Generates random 64x64 matrices with values ranging from 0 to 256 at a frequency of 50Hz.
Sends each generated matrix to appY.
Sends an indication to appUI with the timestamp of the matrix sent to appY.
Key Features:

Independent sending of matrices to appY and timestamps to appUI.
High-frequency matrix generation.
ApplicationY (appY)
Description:

Receives matrices from appX at a frequency of 50Hz.
Filters half of the received matrices (only processes every other matrix).
Transforms each 64x64 matrix with values ranging from 0 to 256 into a 16x16 matrix with values ranging from 0 to 16.
Stores both the source and transformed matrices in memory.
Sends pointers to the source and target matrices to appUI.
Key Features:

Efficient matrix transformation and filtering.
Real-time data processing and storage.
Data Display Application (appUI)
Description:

Receives timestamps from appX at a frequency of 50Hz and prints them.
Receives pointers to source and target matrices from appY at a frequency of 25Hz, loads them, and prints them.
Key Features:

Real-time data display of timestamps and matrices.
Clear and structured data presentation.
System Architecture
The system consists of three main components:

appX: Generates and sends matrices and timestamps.
appY: Receives, filters, transforms, and sends matrices.
appUI: Displays timestamps and matrices.
Installation and Setup
Prerequisites
  C++ compiler (e.g., g++)
  CMake (optional, for building the project)
  Libraries: JSONCPP, libcurl
Building the Project
Clone the repository:
  Copy code
  git clone <repository_url>
  cd <repository_directory>
Compile the applications:

Copy code
  g++ appX.cpp -o appX -ljsoncpp -lcurl
  g++ appY.cpp -o appY -ljsoncpp -lcurl
  g++ appUI.cpp -o appUI -ljsoncpp -lcurl
Running the Applications
Start appY:  
  Copy code
  ./appY
Start appUI:
  Copy code
  ./appUI
Start appX:
  Copy code
  ./appX
#Explanation of Algorithm
Matrix Transformation (Appy-Clause C)
  The algorithm to transform the 64x64 matrix into a 16x16 matrix involves the following steps:
  
  Initialize a 16x16 matrix with zero values.
  For each element in the 64x64 matrix, determine its corresponding position in the 16x16 matrix by dividing the indices by 4.
  Accumulate the values from the 64x64 matrix into the 16x16 matrix.
  Average the accumulated values to fit into the range of 0 to 16.
  Additional Information
  The project is developed using standard C++ and runs on both Windows and Linux environments.
  Detailed documentation is provided within the source code.
Contact
  For any questions or further assistance, please contact the project maintainer.
