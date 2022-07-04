
# Text extractor from images

Simple project aimed to extract text from printed documents. Input image of pdf file, output: text represetntation.

There are 2 projects inside this repository:
1. imb.vcxproj - application with UI itself.
2. imb_test.vcxproj - unit test suite to test the quality of critical functionalities.

[Short video demonstration of this app](https://youtu.be/fcItcY_PNhM)

## Critical things to do:
1. Create cmakelists.txt file to build project under cross-platform compilers.
2. Create project PRO file to build project under QtCreator.
3. Project autodocumentation is completely missing.
4. There is no zoom in/out, pan functionality in app UI for recognition resuts.
5. Binarized comparison function: only 2 first generated images are comparable.

## Code style:

See [coding style](CODINGSTYLE.md) as some desirable code style. This project code is not completely
match to these requirements.


## Code quality checks

### Use clang-tidy
Just run clang-tidy from MSVC. Check setup can be found in ".clang-tidy" file.

or in Visual Studio Developer Command Prompt run:

clang-tidy src/ui/WidImageBinarizer.cpp

clang-tidy src/ui/WidRender.cpp



## Alternative solutions / Things to do

### Only russian language is used as default language
Better to provide language selection in UI, or automatically detect language

### UI enable/disable status
Some UI elements should be disable if applicable

### Pdf rotate option
Can be also applied to ordinary image loaded (jpeg, png, etc. formats)

### Memory leaks check

Memory leak check is active only for debug mode, windows compilation.
It is implemented via crtdbg:

 _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

But it can be implemented in another way

### Pdf read functionality
Implemented via mupdf third-party framework. There are plenty of alternatives. One of them is
QT starting from version 6.3.2

### Pdf scale control
Ideally, application should detect best scale for render source pdf page into image.
In this implementation user can manually select scale in order to try obtain better
recognition results

### Pdf rotate control
Typically in the scanned pdf files image can be oriented in a pseudo random manner,
so engine need to rotate left or right to see normal document page image.
In future this rotation should be auto detected by application/engine.

### Tesseract recognition using
Where to find:

function: WidImageBinarizer::applyTesseract

I have used interface:

Boxa* bounds = ocr->GetWords(NULL);

and iterate box-by-box to obtain detected words.
But also it can be used another interface from mupdf/source/fitz/tessocr.cpp:

ocr_recognise(fz_context *ctx, ...

In this case we can process recognized symbols (not words) one by one,
also detected font metrics will be received into callback to draw
recognized text over source image more precisely.

