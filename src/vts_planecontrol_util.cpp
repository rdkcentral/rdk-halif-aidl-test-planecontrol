/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2025 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
 */

/**
 * @addtogroup Plane_Control Plane Control Module
 * @{
 */

/**
 * @addtogroup Plane_Control_Halif_Test Plane Control HALIF Tests
 * @{
 */

/**
 * @defgroup Plane_Control_Halif_Test_Util Plane Control HALIF Tests util File
 * @{
 * @parblock
 *
 * ### utility functions for Plane Control VTS HALIF AIDL tests :
 *
 *
 *
 * **Pre-Conditions:**  None@n
 * **Dependencies:** None@n
 *
 * Ref to API Definition specification documentation :
 *
 * @endparblock
 */


#include "vts_planecontrol.h"
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <cstdio>

static bool readNextPpmToken(std::istream& in, std::string& token)
{
    token.clear();
    while (in >> token)
    {
        if (!token.empty() && token[0] == '#')
        {
            std::string discard;
            std::getline(in, discard);
            continue;
        }
        return true;
    }
    return false;
}

static bool isSupportedRasterImagePath(const std::string& path)
{
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return (ext == ".ppm" || ext == ".png" || ext == ".jpg" || ext == ".jpeg");
}

static std::string shellEscapeSingleQuotes(const std::string& input)
{
    std::string out;
    out.reserve(input.size() + 8);
    for (char c : input)
    {
        if (c == '\'')
        {
            out += "'\\''";
        }
        else
        {
            out += c;
        }
    }
    return out;
}

static bool convertImageToPpmIfNeeded(const std::string& inputPath, std::string& ppmPathOut)
{
    ppmPathOut.clear();
    if (inputPath.empty())
    {
        return false;
    }

    std::string ext = std::filesystem::path(inputPath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".ppm")
    {
        ppmPathOut = inputPath;
        return true;
    }

    if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
    {
        UT_LOG_INFO("Unsupported image extension for conversion: %s", inputPath.c_str());
        return false;
    }

    const std::filesystem::path outPath = std::filesystem::temp_directory_path() /
                                          ("vts_plane_img_" + std::to_string(getpid()) + ".ppm");
    const std::string escapedIn = shellEscapeSingleQuotes(inputPath);
    const std::string escapedOut = shellEscapeSingleQuotes(outPath.string());

    const std::string magickCmd = "magick '" + escapedIn + "' -compress none '" + escapedOut + "'";
    int rc = std::system(magickCmd.c_str());
    if (rc != 0)
    {
        const std::string convertCmd = "convert '" + escapedIn + "' -compress none '" + escapedOut + "'";
        rc = std::system(convertCmd.c_str());
    }

    if (rc != 0 || !std::filesystem::exists(outPath))
    {
        UT_LOG_INFO("Failed to convert image to PPM. Install ImageMagick (magick/convert). Input: %s", inputPath.c_str());
        return false;
    }

    ppmPathOut = outPath.string();
    return true;
}

static bool loadPpmP6Image(const std::string& path, int& width, int& height, std::vector<uint8_t>& rgb)
{
    std::ifstream input(path, std::ios::binary);
    if (!input.is_open())
    {
        UT_LOG_INFO("Failed to open image file: %s", path.c_str());
        return false;
    }

    std::string magic;
    std::string token;
    if (!readNextPpmToken(input, magic) || (magic != "P6" && magic != "P3"))
    {
        UT_LOG_INFO("Unsupported PPM format in %s. Expected P6 or P3.", path.c_str());
        return false;
    }
    if (!readNextPpmToken(input, token))
    {
        UT_LOG_INFO("Failed to read width from image: %s", path.c_str());
        return false;
    }
    try
    {
        width = std::stoi(token);
    }
    catch (...)
    {
        UT_LOG_INFO("Invalid width token in image: %s", path.c_str());
        return false;
    }
    if (!readNextPpmToken(input, token))
    {
        UT_LOG_INFO("Failed to read height from image: %s", path.c_str());
        return false;
    }
    try
    {
        height = std::stoi(token);
    }
    catch (...)
    {
        UT_LOG_INFO("Invalid height token in image: %s", path.c_str());
        return false;
    }
    if (!readNextPpmToken(input, token))
    {
        UT_LOG_INFO("Failed to read max value from image: %s", path.c_str());
        return false;
    }
    int maxVal = 0;
    try
    {
        maxVal = std::stoi(token);
    }
    catch (...)
    {
        UT_LOG_INFO("Invalid max value token in image: %s", path.c_str());
        return false;
    }
    if (width <= 0 || height <= 0 || maxVal != 255)
    {
        UT_LOG_INFO("Unsupported image dimensions/max value in %s (w=%d h=%d max=%d)",
                    path.c_str(), width, height, maxVal);
        return false;
    }

    const size_t bytes = static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
    rgb.resize(bytes);
    if (magic == "P6")
    {
        input >> std::ws;
        input.read(reinterpret_cast<char*>(rgb.data()), static_cast<std::streamsize>(bytes));
        const bool ok = input.good() || input.gcount() == static_cast<std::streamsize>(bytes);
        if (!ok)
        {
            UT_LOG_INFO("Failed to read full P6 pixel data from image: %s", path.c_str());
        }
        return ok;
    }

    for (size_t i = 0; i < bytes; ++i)
    {
        if (!readNextPpmToken(input, token))
        {
            UT_LOG_INFO("Failed to read full P3 pixel data from image: %s", path.c_str());
            return false;
        }
        int value = 0;
        try
        {
            value = std::stoi(token);
        }
        catch (...)
        {
            UT_LOG_INFO("Invalid P3 pixel token in image: %s", path.c_str());
            return false;
        }
        if (value < 0 || value > 255)
        {
            UT_LOG_INFO("P3 pixel out of range in image: %s", path.c_str());
            return false;
        }
        rgb[i] = static_cast<uint8_t>(value);
    }
    return true;
}

static bool drawPpmImageToDmaBuf(
    const std::string& path,
    uint8_t* base, int32_t stride, int32_t pixelWidth, int32_t pixelHeight, int32_t offset)
{
    std::string ppmPath;
    if (!convertImageToPpmIfNeeded(path, ppmPath))
    {
        return false;
    }

    int srcW = 0;
    int srcH = 0;
    std::vector<uint8_t> srcRgb;
    if (!loadPpmP6Image(ppmPath, srcW, srcH, srcRgb))
    {
        return false;
    }

    uint8_t* pixels = base + offset;
    for (int32_t y = 0; y < pixelHeight; ++y)
    {
        uint8_t* row = pixels + static_cast<size_t>(y) * static_cast<size_t>(stride);
        const int srcY = static_cast<int>((static_cast<int64_t>(y) * srcH) / pixelHeight);
        for (int32_t x = 0; x < pixelWidth; ++x)
        {
            const int srcX = static_cast<int>((static_cast<int64_t>(x) * srcW) / pixelWidth);
            const size_t srcIdx = (static_cast<size_t>(srcY) * static_cast<size_t>(srcW) + static_cast<size_t>(srcX)) * 3;
            uint8_t* dst = row + x * 4;
            dst[0] = srcRgb[srcIdx + 2];
            dst[1] = srcRgb[srcIdx + 1];
            dst[2] = srcRgb[srcIdx + 0];
            dst[3] = 0xFF;
        }
    }

    return true;
}

bool askUserYesNo(const std::string& question)
{
    while (true)
    {
        UT_LOG_INFO("%s (yes/no): ", question.c_str());

        std::string input;
        if (!std::getline(std::cin, input))
        {
            UT_LOG_INFO("No user input available (EOF)");
            return false;
        }

        const auto first = input.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
        {
            input.clear();
        }
        else
        {
            const auto last = input.find_last_not_of(" \t\n\r");
            input = input.substr(first, last - first + 1);
        }

        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        if (input == "yes" || input == "y")
        {
            UT_LOG_INFO("User selected YES");
            return true;
        }
        if (input == "no" || input == "n")
        {
            UT_LOG_INFO("User selected NO");
            return false;
        }

        UT_LOG_INFO("Invalid input. Please type \"yes\" or \"no\".");
    }
}

bool isGraphicsPlaneTestName(const std::string& testName)
{
    return (testName == "Planecontrol_Graphicsplane" ||
            testName == "Planecontrol_Graphicsplane_Auto" ||
            testName == "Planecontrol_Graphicsplane_Client");
}

bool isGraphicsPlaneAutoTest(const std::string& testName)
{
    return testName == "Planecontrol_Graphicsplane_Auto";
}

bool isGraphicsPlaneClientTest(const std::string& testName)
{
    return testName == "Planecontrol_Graphicsplane_Client";
}

size_t getGraphicsFbMapLength(const ::com::rdk::hal::planecontrol::GraphicsFbInfo& fbInfo)
{
    return static_cast<size_t>(fbInfo.offset) +
           static_cast<size_t>(fbInfo.stride) * static_cast<size_t>(fbInfo.pixelHeight);
}

void verifyTriangleSpotPixels(
    uint8_t* base, int32_t stride, int32_t pixelWidth, int32_t pixelHeight, int32_t offset,
    uint8_t r, uint8_t g, uint8_t b)
{
    auto pixelAt = [&](int32_t x, int32_t y) -> uint8_t* {
        return base + offset + static_cast<size_t>(y) * static_cast<size_t>(stride) + static_cast<size_t>(x) * 4;
    };

    const int32_t inX = pixelWidth / 2;
    const int32_t inY = (pixelHeight * 7) / 10;
    const int32_t outX = pixelWidth / 20;
    const int32_t outY = pixelHeight / 20;

    uint8_t* inPixel = pixelAt(inX, inY);
    UT_ASSERT_EQUAL(inPixel[0], b);
    UT_ASSERT_EQUAL(inPixel[1], g);
    UT_ASSERT_EQUAL(inPixel[2], r);
    UT_ASSERT_EQUAL(inPixel[3], 0xFF);

    uint8_t* outPixel = pixelAt(outX, outY);
    UT_ASSERT_EQUAL(outPixel[0], 0x00);
    UT_ASSERT_EQUAL(outPixel[1], 0x00);
    UT_ASSERT_EQUAL(outPixel[2], 0x00);
    UT_ASSERT_EQUAL(outPixel[3], 0xFF);
}

std::string getGraphicsClientImagePath()
{
    static bool resolved = false;
    static std::string resolvedPath;
    if (resolved)
    {
        return resolvedPath;
    }

    const char* imageEnv = std::getenv("VTS_PLANE_CLIENT_IMAGE");
    if (imageEnv != nullptr && imageEnv[0] != '\0')
    {
        resolvedPath = std::string(imageEnv);
        resolved = true;
        return resolvedPath;
    }

    try
    {
        const std::filesystem::path cwd = std::filesystem::current_path();
        for (const auto& entry : std::filesystem::recursive_directory_iterator(
                 cwd, std::filesystem::directory_options::skip_permission_denied))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".ppm" || ext == ".png" || ext == ".jpg" || ext == ".jpeg")
            {
                resolvedPath = entry.path().string();
                UT_LOG_INFO("VTS_PLANE_CLIENT_IMAGE not set. Using image found under current directory: %s",
                            resolvedPath.c_str());
                break;
            }
        }

        if (resolvedPath.empty())
        {
            UT_LOG_INFO("VTS_PLANE_CLIENT_IMAGE not set and no supported image (.ppm/.png/.jpg/.jpeg) found under current directory: %s",
                        cwd.string().c_str());
        }
    }
    catch (const std::exception& ex)
    {
        UT_LOG_INFO("Unable to scan current directory for .ppm image: %s", ex.what());
    }

    resolved = true;
    return resolvedPath;
}

bool hasGraphicsClientImagePath()
{
    return !getGraphicsClientImagePath().empty();
}

GraphicsClientPattern getGraphicsClientPattern()
{
    const char* patternEnv = std::getenv("VTS_PLANE_CLIENT_PATTERN");
    if (patternEnv == nullptr)
    {
        return GraphicsClientPattern::TRIANGLE;
    }

    std::string pattern(patternEnv);
    std::transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);

    if (pattern == "rectangle" || pattern == "rect")
    {
        return GraphicsClientPattern::RECTANGLE;
    }
    if (pattern == "solid")
    {
        return GraphicsClientPattern::SOLID;
    }
    if (pattern == "yellow" || pattern == "solid_yellow")
    {
        return GraphicsClientPattern::SOLID_YELLOW;
    }
    if (pattern == "image")
    {
        return GraphicsClientPattern::IMAGE;
    }

    return GraphicsClientPattern::TRIANGLE;
}

const char* getGraphicsClientPatternLabel(GraphicsClientPattern pattern)
{
    switch (pattern)
    {
        case GraphicsClientPattern::RECTANGLE:
            return "rectangle";
        case GraphicsClientPattern::SOLID:
            return "solid color screen";
        case GraphicsClientPattern::SOLID_YELLOW:
            return "solid yellow screen";
        case GraphicsClientPattern::IMAGE:
            return "image";
        case GraphicsClientPattern::TRIANGLE:
        default:
            return "triangle";
    }
}

void drawTriangleToDmaBuf(
    uint8_t* base, int32_t stride, int32_t pixelWidth, int32_t pixelHeight, int32_t offset,
    uint8_t r, uint8_t g, uint8_t b)
{
    const int32_t v0x = pixelWidth / 2,          v0y = pixelHeight / 10;
    const int32_t v1x = pixelWidth / 10,         v1y = (pixelHeight * 9) / 10;
    const int32_t v2x = (pixelWidth * 9) / 10,   v2y = (pixelHeight * 9) / 10;

    uint8_t* pixels = base + offset;
    for (int32_t py = 0; py < pixelHeight; ++py)
    {
        uint8_t* row = pixels + static_cast<size_t>(py) * static_cast<size_t>(stride);
        for (int32_t px = 0; px < pixelWidth; ++px)
        {
            const int64_t e0 = (int64_t)(px - v0x) * (v1y - v0y) - (int64_t)(py - v0y) * (v1x - v0x);
            const int64_t e1 = (int64_t)(px - v1x) * (v2y - v1y) - (int64_t)(py - v1y) * (v2x - v1x);
            const int64_t e2 = (int64_t)(px - v2x) * (v0y - v2y) - (int64_t)(py - v2y) * (v0x - v2x);

            uint8_t* pixel = row + px * 4;
            if (e0 >= 0 && e1 >= 0 && e2 >= 0)
            {
                pixel[0] = b;
                pixel[1] = g;
                pixel[2] = r;
                pixel[3] = 0xFF;
            }
            else
            {
                pixel[0] = 0x00;
                pixel[1] = 0x00;
                pixel[2] = 0x00;
                pixel[3] = 0xFF;
            }
        }
    }
}

void fillSolidColorToDmaBuf(
    uint8_t* base, int32_t stride, int32_t pixelWidth, int32_t pixelHeight, int32_t offset,
    uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t* pixels = base + offset;
    for (int32_t py = 0; py < pixelHeight; ++py)
    {
        uint8_t* row = pixels + static_cast<size_t>(py) * static_cast<size_t>(stride);
        for (int32_t px = 0; px < pixelWidth; ++px)
        {
            uint8_t* pixel = row + px * 4;
            pixel[0] = b;
            pixel[1] = g;
            pixel[2] = r;
            pixel[3] = 0xFF;
        }
    }
}

void drawRectangleToDmaBuf(
    uint8_t* base, int32_t stride, int32_t pixelWidth, int32_t pixelHeight, int32_t offset,
    uint8_t r, uint8_t g, uint8_t b)
{
    const int32_t x0 = pixelWidth / 5;
    const int32_t x1 = (pixelWidth * 4) / 5;
    const int32_t y0 = pixelHeight / 5;
    const int32_t y1 = (pixelHeight * 4) / 5;

    uint8_t* pixels = base + offset;
    for (int32_t py = 0; py < pixelHeight; ++py)
    {
        uint8_t* row = pixels + static_cast<size_t>(py) * static_cast<size_t>(stride);
        for (int32_t px = 0; px < pixelWidth; ++px)
        {
            const bool inRect = (px >= x0 && px <= x1 && py >= y0 && py <= y1);
            uint8_t* pixel = row + px * 4;
            if (inRect)
            {
                pixel[0] = b;
                pixel[1] = g;
                pixel[2] = r;
                pixel[3] = 0xFF;
            }
            else
            {
                pixel[0] = 0x00;
                pixel[1] = 0x00;
                pixel[2] = 0x00;
                pixel[3] = 0xFF;
            }
        }
    }
}

void drawClientPatternToDmaBuf(
    GraphicsClientPattern pattern,
    uint8_t* base, int32_t stride, int32_t pixelWidth, int32_t pixelHeight, int32_t offset,
    uint8_t r, uint8_t g, uint8_t b)
{
    if (pattern == GraphicsClientPattern::IMAGE)
    {
        const std::string imagePath = getGraphicsClientImagePath();
        if (!imagePath.empty() && isSupportedRasterImagePath(imagePath) &&
            drawPpmImageToDmaBuf(imagePath, base, stride, pixelWidth, pixelHeight, offset))
        {
            UT_LOG_INFO("Rendered image pattern from: %s", imagePath.c_str());
            return;
        }
        UT_LOG_INFO("Image mode selected, but image load failed for path: %s. Falling back to solid yellow screen.",
                    imagePath.empty() ? "<none>" : imagePath.c_str());
        fillSolidColorToDmaBuf(base, stride, pixelWidth, pixelHeight, offset, 255, 255, 0);
        return;
    }

    if (pattern == GraphicsClientPattern::SOLID_YELLOW)
    {
        fillSolidColorToDmaBuf(base, stride, pixelWidth, pixelHeight, offset, 255, 255, 0);
        return;
    }

    if (pattern == GraphicsClientPattern::SOLID)
    {
        fillSolidColorToDmaBuf(base, stride, pixelWidth, pixelHeight, offset, r, g, b);
        return;
    }

    if (pattern == GraphicsClientPattern::RECTANGLE)
    {
        drawRectangleToDmaBuf(base, stride, pixelWidth, pixelHeight, offset, r, g, b);
        return;
    }

    drawTriangleToDmaBuf(base, stride, pixelWidth, pixelHeight, offset, r, g, b);
}


// Global vector to store parsed plane configurations
std::vector<planeControl_Cap> gPlaneControl_hfp;

extern sp<IPlaneControl> gPlaneControlservice;
// Conversion functions (assumed to exist)

std::string planeTypeToString(::com::rdk::hal::planecontrol::PlaneType type) {
    switch (type) {
        case ::com::rdk::hal::planecontrol::PlaneType::VIDEO: return "VIDEO";
        case ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS: return "GRAPHICS";
        default: return "UNKNOWN";
    }
}

std::string pixelFormatToString(::com::rdk::hal::videodecoder::PixelFormat format) {
    switch (format) {
        case ::com::rdk::hal::videodecoder::PixelFormat::YCBCR422: return "YCBCR422";
        case ::com::rdk::hal::videodecoder::PixelFormat::YCBCR444: return "YCBCR444";
        case ::com::rdk::hal::videodecoder::PixelFormat::YCBCR420: return "YCBCR420";
        case ::com::rdk::hal::videodecoder::PixelFormat::RGB: return "RGB";
        case ::com::rdk::hal::videodecoder::PixelFormat::NATIVE: return "NATIVE";
        default: return "UNKNOWN";
    }
}

std::string sourceTypeToString(::com::rdk::hal::planecontrol::SourceType type) {
    switch (type) {
        case ::com::rdk::hal::planecontrol::SourceType::NONE: return "NONE";
        case ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK: return "VIDEO_SINK";
        case ::com::rdk::hal::planecontrol::SourceType::HDMI: return "HDMI";
        case ::com::rdk::hal::planecontrol::SourceType::COMPOSITE: return "COMPOSITE";
        default: return "UNKNOWN";
    }
}

std::string dynamicRangeToString(::com::rdk::hal::videodecoder::DynamicRange range) {
    switch (range) {
        case ::com::rdk::hal::videodecoder::DynamicRange::UNKNOWN: return "UNKNOWN";
        case ::com::rdk::hal::videodecoder::DynamicRange::SDR: return "SDR";
        case ::com::rdk::hal::videodecoder::DynamicRange::HLG: return "HLG";
        case ::com::rdk::hal::videodecoder::DynamicRange::HDR10: return "HDR10";
        case ::com::rdk::hal::videodecoder::DynamicRange::HDR10_PLUS: return "HDR10_PLUS";
        case ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION: return "DOLBY_VISION";
        default: return "UNKNOWN";
    }
}

::com::rdk::hal::planecontrol::PlaneType stringToPlaneType(const std::string& typeStr) {
    if (typeStr == "VIDEO") return ::com::rdk::hal::planecontrol::PlaneType::VIDEO;
    if (typeStr == "GRAPHICS") return ::com::rdk::hal::planecontrol::PlaneType::GRAPHICS;
    throw std::invalid_argument("Unknown plane type: " + typeStr);
}

::com::rdk::hal::videodecoder::PixelFormat stringToPixelFormat(const std::string& str) {
    if (str == "YCBCR422") return ::com::rdk::hal::videodecoder::PixelFormat::YCBCR422;
    if (str == "YCBCR444") return ::com::rdk::hal::videodecoder::PixelFormat::YCBCR444;
    if (str == "YCBCR420") return ::com::rdk::hal::videodecoder::PixelFormat::YCBCR420;
    if (str == "RGB") return ::com::rdk::hal::videodecoder::PixelFormat::RGB;
    if (str == "NATIVE") return ::com::rdk::hal::videodecoder::PixelFormat::NATIVE;
    throw std::invalid_argument("Invalid pixel format: " + str);
}

::com::rdk::hal::videodecoder::DynamicRange stringToDynamicRange(const std::string& drStr) {
    if (drStr == "UNKNOWN") return ::com::rdk::hal::videodecoder::DynamicRange::UNKNOWN;
    if (drStr == "SDR") return ::com::rdk::hal::videodecoder::DynamicRange::SDR;
    if (drStr == "HLG") return ::com::rdk::hal::videodecoder::DynamicRange::HLG;
    if (drStr == "HDR10") return ::com::rdk::hal::videodecoder::DynamicRange::HDR10;
    if (drStr == "HDR10_PLUS") return ::com::rdk::hal::videodecoder::DynamicRange::HDR10_PLUS;
    if (drStr == "DOLBY_VISION") return ::com::rdk::hal::videodecoder::DynamicRange::DOLBY_VISION;
    throw std::invalid_argument("Unknown dynamic range: " + drStr);
}

::com::rdk::hal::planecontrol::SourceType stringToSourceType(const std::string& str) {
    if (str == "NONE") return ::com::rdk::hal::planecontrol::SourceType::NONE;
    if (str == "VIDEO_SINK") return ::com::rdk::hal::planecontrol::SourceType::VIDEO_SINK;
    if (str == "HDMI") return ::com::rdk::hal::planecontrol::SourceType::HDMI;
    if (str == "COMPOSITE") return ::com::rdk::hal::planecontrol::SourceType::COMPOSITE;
    throw std::invalid_argument("Invalid source type: " + str);
}

UT_status_t getPlaneControlService() {
    gPlaneControlservice = android::interface_cast<IPlaneControl>(
            defaultServiceManager()->getService(String16(IPlaneControl::serviceName().c_str())));
    if (gPlaneControlservice == nullptr) {
        printf("PlaneControl Service not available\n");
        return UT_STATUS_FAILURE;
    }
    return UT_STATUS_OK;
}


// Helper function to validate property lists for VIDEO or GRAPHICS planes
bool ValidatePropertyList(uint32_t planeIndex, const std::vector<::com::rdk::hal::planecontrol::Property>& expectedProperties, const std::string& planeType) {
    std::vector<::com::rdk::hal::planecontrol::PropertyKVPair> propertyKVList;

    bool aidl_return = false;
    std::string inputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], IN:properties.size:[" + std::to_string(expectedProperties.size()) + "], OUT:propertyKVList[], OUT:_aidl_return";
    auto status = gPlaneControlservice->getPropertyMulti(planeIndex, expectedProperties, &propertyKVList, &aidl_return);

    std::string outputParams = "IN:planeResourceIndex:[" + std::to_string(planeIndex) + "], OUT:propertyKVList.size:[" + std::to_string(propertyKVList.size()) + "], OUT:_aidl_return:[" + (aidl_return ? "true" : "false") + "]";
    //UT_LOG_INFO("getPropertyMulti with " + planeType + " plane index", inputParams, outputParams, status, aidl_return);

    bool success = true;
    if (!status.isOk()) {
        success = false;
        EXPECT_TRUE(status.isOk()) << "Status check failed for getPropertyMulti";
    }
    if (!aidl_return) {
        success = false;
        EXPECT_TRUE(aidl_return) << "aidl_return is false for getPropertyMulti";
    }
    if (propertyKVList.empty()) {
        success = false;
        EXPECT_FALSE(propertyKVList.empty()) << "propertyKVList is empty";
    }
    if (propertyKVList.size() != expectedProperties.size()) {
        success = false;
        EXPECT_EQ(propertyKVList.size(), expectedProperties.size()) << "propertyKVList size mismatch";
    }

    for (const auto& kvPair : propertyKVList) {
        if (!kvPair.propertyValue.value.has_value()) {
            success = false;
            EXPECT_TRUE(kvPair.propertyValue.value.has_value()) << "Property value is not set";
        }
    }

    return success;
}

UT_status_t vts_planecontrol_parsePlaneControlHFP() {
    const std::string planeControlPath = PLANE_CONTROL_MGR_PLANES;
    int32_t number_of_planes = 0;

    // Reset global counters
    gNoVideoTypePlanes = 0;
    gNoGraphicsTypePlanes = 0;

    // Verify KVP instance
    ut_kvp_instance_t* kvp_instance = ut_kvp_profile_getInstance();
    if (!kvp_instance) {
        UT_LOG_ERROR("Failed to get KVP profile instance");
        return UT_STATUS_FAILURE;
    }

    UT_LOG_INFO("Parsing HFP from: %s", planeControlPath.c_str());

    // Get number of planes
    number_of_planes = UT_KVP_PROFILE_GET_LIST_COUNT(planeControlPath.c_str());
    UT_LOG_INFO("Number of planes: %d", number_of_planes);
    if (number_of_planes <= 0) {
        UT_LOG_ERROR("No planes found in HFP at path: %s", planeControlPath.c_str());
        return UT_STATUS_FAILURE;
    }

    gPlaneControl_hfp.clear();
    for (int32_t i = 0; i < number_of_planes; ++i) {
        std::string index = std::to_string(i);
        // YAML structure: planes is a list, each item is a dict with numeric key
        // Path: planecontrol.planes.0.0 (list index 0, dict key "0")
        std::string planeBasePath = planeControlPath + "." + index + "." + index;
        std::string typePath = planeBasePath + ".type";
        std::string pixelFormatsPath = planeBasePath + ".pixelFormats";
        std::string dynamicRangesPath = planeBasePath + ".supportedDynamicRanges";

        planeControl_Cap cap;
        cap.planeIndex = i;

        // Read plane type
        std::string result_kvp(UT_KVP_MAX_ELEMENT_SIZE, '\0');
        ut_kvp_status_t status = ut_kvp_getStringField(kvp_instance, typePath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
        if (status != UT_KVP_STATUS_SUCCESS) {
            UT_LOG_WARNING("Failed to read plane type at path: %s, status: %d, skipping plane %d",
                           typePath.c_str(), status, i);
            continue;
        }
        result_kvp.resize(std::strlen(result_kvp.c_str()));
        UT_LOG_DEBUG("Reading plane type path: %s, value: %s", typePath.c_str(), result_kvp.c_str());
        if (result_kvp.empty()) {
            UT_LOG_WARNING("Empty plane type at path: %s, skipping plane %d", typePath.c_str(), i);
            continue;
        }

        // Update global counters based on plane type
        if (result_kvp == "VIDEO") {
            gNoVideoTypePlanes++;
        } else if (result_kvp == "GRAPHICS") {
            gNoGraphicsTypePlanes++;
        }

        try {
            cap.type = stringToPlaneType(result_kvp);
        } catch (const std::invalid_argument& e) {
            UT_LOG_WARNING("Invalid plane type at path: %s, value: %s, error: %s",
                           typePath.c_str(), result_kvp.c_str(), e.what());
            continue;
        }

        // Read pixel formats
        int32_t pixel_format_count = UT_KVP_PROFILE_GET_LIST_COUNT(pixelFormatsPath.c_str());
        UT_LOG_INFO("Plane %d: pixel format count=%d", i, pixel_format_count);
        cap.pixelFormats.clear();
        for (int32_t j = 0; j < pixel_format_count; ++j) {
            std::string formatPath = pixelFormatsPath + "." + std::to_string(j);
            result_kvp.resize(UT_KVP_MAX_ELEMENT_SIZE);
            status = ut_kvp_getStringField(kvp_instance, formatPath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
            if (status != UT_KVP_STATUS_SUCCESS) {
                UT_LOG_WARNING("Failed to read pixel format at path: %s, status: %d", formatPath.c_str(), status);
                continue;
            }
            result_kvp.resize(std::strlen(result_kvp.c_str()));
            UT_LOG_DEBUG("Reading pixel format path: %s, value: %s", formatPath.c_str(), result_kvp.c_str());
            if (result_kvp.empty()) {
                UT_LOG_WARNING("Empty pixel format at path: %s", formatPath.c_str());
                continue;
            }
            try {
                cap.pixelFormats.push_back(stringToPixelFormat(result_kvp));
            } catch (const std::invalid_argument& e) {
                UT_LOG_WARNING("Invalid pixel format at path: %s, value: %s, error: %s",
                               formatPath.c_str(), result_kvp.c_str(), e.what());
            }
        }
        if (cap.pixelFormats.empty()) {
            UT_LOG_WARNING("Plane %d has no valid pixel formats, skipping", i);
            continue;
        }

        // Read source types (optional field)
        std::string sourceTypesPath = planeBasePath + ".sourceTypes";
        int32_t source_type_count = 0;
        if (ut_kvp_fieldPresent(kvp_instance, sourceTypesPath.c_str())) {
            source_type_count = UT_KVP_PROFILE_GET_LIST_COUNT(sourceTypesPath.c_str());
        }
        UT_LOG_INFO("Plane %d: source type count=%d", i, source_type_count);
        cap.sourceTypes.clear();
        if (source_type_count > 0) {
            for (int32_t j = 0; j < source_type_count; ++j) {
                std::string srcTypePath = sourceTypesPath + "." + std::to_string(j);
                result_kvp.resize(UT_KVP_MAX_ELEMENT_SIZE);
                status = ut_kvp_getStringField(kvp_instance, srcTypePath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
                if (status != UT_KVP_STATUS_SUCCESS) {
                    UT_LOG_WARNING("Failed to read source type at path: %s, status: %d", srcTypePath.c_str(), status);
                    continue;
                }
                result_kvp.resize(std::strlen(result_kvp.c_str()));
                UT_LOG_DEBUG("Reading source type path: %s, value: %s", srcTypePath.c_str(), result_kvp.c_str());
                if (result_kvp.empty()) {
                    UT_LOG_WARNING("Empty source type at path: %s", srcTypePath.c_str());
                    continue;
                }
                try {
                    cap.sourceTypes.push_back(stringToSourceType(result_kvp));
                } catch (const std::invalid_argument& e) {
                    UT_LOG_WARNING("Invalid source type at path: %s, value: %s, error: %s",
                                   srcTypePath.c_str(), result_kvp.c_str(), e.what());
                }
            }
        } else {
            UT_LOG_INFO("Plane %d: No sourceTypes defined in profile (optional field)", i);
        }

        // Read supported dynamic ranges
        int32_t dynamic_range_count = UT_KVP_PROFILE_GET_LIST_COUNT(dynamicRangesPath.c_str());
        UT_LOG_INFO("Plane %d: dynamic range count=%d", i, dynamic_range_count);
        cap.supportedDynamicRanges.clear();
        for (int32_t j = 0; j < dynamic_range_count; ++j) {
            std::string drPath = dynamicRangesPath + "." + std::to_string(j);
            result_kvp.resize(UT_KVP_MAX_ELEMENT_SIZE);
            status = ut_kvp_getStringField(kvp_instance, drPath.c_str(), result_kvp.data(), UT_KVP_MAX_ELEMENT_SIZE);
            if (status != UT_KVP_STATUS_SUCCESS) {
                UT_LOG_WARNING("Failed to read dynamic range at path: %s, status: %d", drPath.c_str(), status);
                continue;
            }
            result_kvp.resize(std::strlen(result_kvp.c_str()));
            UT_LOG_DEBUG("Reading dynamic range path: %s, value: %s", drPath.c_str(), result_kvp.c_str());
            if (result_kvp.empty()) {
                UT_LOG_WARNING("Empty dynamic range at path: %s", drPath.c_str());
                continue;
            }
            try {
                cap.supportedDynamicRanges.push_back(stringToDynamicRange(result_kvp));
            } catch (const std::invalid_argument& e) {
                UT_LOG_WARNING("Invalid dynamic range at path: %s, value: %s, error: %s",
                               drPath.c_str(), result_kvp.c_str(), e.what());
            }
        }
        if (cap.supportedDynamicRanges.empty()) {
            UT_LOG_WARNING("Plane %d has no valid dynamic ranges, skipping", i);
            continue;
        }

        // Read maxWidth, maxHeight, frameWidth, frameHeight
        cap.maxWidth = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".maxWidth").c_str());
        cap.maxHeight = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".maxHeight").c_str());
        cap.frameWidth = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".frameWidth").c_str());
        cap.frameHeight = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".frameHeight").c_str());
        UT_LOG_DEBUG("Plane %d: maxWidth=%d, maxHeight=%d, frameWidth=%d, frameHeight=%d",
                     i, cap.maxWidth, cap.maxHeight, cap.frameWidth, cap.frameHeight);
        if (cap.maxWidth <= 0 || cap.maxHeight <= 0) {
            UT_LOG_WARNING("Invalid maxWidth=%d or maxHeight=%d for plane %d, skipping",
                           cap.maxWidth, cap.maxHeight, i);
            continue;
        }

        // Read other properties
        cap.colorDepth = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".colorDepth").c_str());
        cap.maxFrameRate = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".maxFrameRate").c_str());
        cap.supportsAlpha = UT_KVP_PROFILE_GET_BOOL((planeBasePath + ".supportsAlpha").c_str());
        cap.supportsZOrder = UT_KVP_PROFILE_GET_BOOL((planeBasePath + ".supportsZOrder").c_str());
        cap.vsyncDisplayLatency = UT_KVP_PROFILE_GET_UINT32((planeBasePath + ".vsyncDisplayLatency").c_str());
        UT_LOG_DEBUG("Plane %d: colorDepth=%d, maxFrameRate=%d, supportsAlpha=%s, supportsZOrder=%s, vsyncDisplayLatency=%d",
                     i, cap.colorDepth, cap.maxFrameRate,
                     cap.supportsAlpha ? "true" : "false",
                     cap.supportsZOrder ? "true" : "false",
                     cap.vsyncDisplayLatency);

        UT_LOG_INFO("Plane %d: index=%d, type=%d, %zu pixel formats, %zu source types, %zu dynamic ranges, colorDepth=%d, maxFrameRate=%d",
                    i, cap.planeIndex, static_cast<int>(cap.type), cap.pixelFormats.size(),
                    cap.sourceTypes.size(), cap.supportedDynamicRanges.size(), cap.colorDepth, cap.maxFrameRate);

        gPlaneControl_hfp.push_back(cap);
    }

    if (gPlaneControl_hfp.empty()) {
        UT_LOG_ERROR("No valid planes parsed from HFP");
        return UT_STATUS_FAILURE;
    }

    UT_LOG_INFO("Successfully parsed %zu planes, VIDEO planes: %u, GRAPHICS planes: %u",
                gPlaneControl_hfp.size(), gNoVideoTypePlanes, gNoGraphicsTypePlanes);
    return UT_STATUS_OK;
}

void validateCapabilityMatch(const ::com::rdk::hal::planecontrol::PlaneCapabilities& actual,
    const planeControl_Cap& expected) {
    // Verify planeIndex
    UT_ASSERT_EQUAL(actual.planeIndex, expected.planeIndex) << "Plane index mismatch";

    // Verify type
    UT_ASSERT_EQUAL(actual.type, expected.type) << "Plane type mismatch, expected: "
                        << planeTypeToString(expected.type)
                        << ", got: " << planeTypeToString(actual.type);

    // Verify pixelFormats
    auto expected_pixel_formats = expected.pixelFormats;
    auto actual_pixel_formats = actual.pixelFormats;
    std::sort(expected_pixel_formats.begin(), expected_pixel_formats.end());
    std::sort(actual_pixel_formats.begin(), actual_pixel_formats.end());
    UT_ASSERT_EQUAL(expected_pixel_formats.size(), actual_pixel_formats.size()) << "Pixel format count mismatch";
    bool pixel_formats_match = std::equal(expected_pixel_formats.begin(), expected_pixel_formats.end(), actual_pixel_formats.begin());
    if (!pixel_formats_match) {
    std::string expected_str, actual_str;
    for (const auto& pf : expected_pixel_formats) expected_str += pixelFormatToString(pf) + ",";
    for (const auto& pf : actual_pixel_formats) actual_str += pixelFormatToString(pf) + ",";
    UT_FAIL("Pixel formats mismatch for plane " << actual.planeIndex
                            << ", expected: [" << expected_str << "]"
                            << ", got: [" << actual_str << "]");
    }

    // Verify sourceTypes
    auto expected_source_types = expected.sourceTypes;
    auto actual_source_types = actual.sourceTypes;
    std::sort(expected_source_types.begin(), expected_source_types.end());
    std::sort(actual_source_types.begin(), actual_source_types.end());
    UT_ASSERT_EQUAL(expected_source_types.size(), actual_source_types.size()) << "Source type count mismatch";
    bool source_types_match = std::equal(expected_source_types.begin(), expected_source_types.end(), actual_source_types.begin());
    if (!source_types_match) {
        std::string expected_str, actual_str;
        for (const auto& st : expected_source_types) expected_str += sourceTypeToString(st) + ",";
        for (const auto& st : actual_source_types) actual_str += sourceTypeToString(st) + ",";
        UT_FAIL("Source types mismatch for plane " << actual.planeIndex
                                << ", expected: [" << expected_str << "]"
                                << ", got: [" << actual_str << "]");
    }

    // Verify supportedDynamicRanges
    auto expected_dynamic_ranges = expected.supportedDynamicRanges;
    auto actual_dynamic_ranges = actual.supportedDynamicRanges;
    std::sort(expected_dynamic_ranges.begin(), expected_dynamic_ranges.end());
    std::sort(actual_dynamic_ranges.begin(), actual_dynamic_ranges.end());
    UT_ASSERT_EQUAL(expected_dynamic_ranges.size(), actual_dynamic_ranges.size()) << "Dynamic range count mismatch";
    bool dynamic_ranges_match = std::equal(expected_dynamic_ranges.begin(), expected_dynamic_ranges.end(), actual_dynamic_ranges.begin());
    if (!dynamic_ranges_match) {
    std::string expected_str, actual_str;
    for (const auto& dr : expected_dynamic_ranges) expected_str += dynamicRangeToString(dr) + ",";
    for (const auto& dr : actual_dynamic_ranges) actual_str += dynamicRangeToString(dr) + ",";
    UT_FAIL("Dynamic ranges mismatch for plane " << actual.planeIndex
                                << ", expected: [" << expected_str << "]"
                                << ", got: [" << actual_str << "]");
    }

    // Verify other fields
    UT_ASSERT_EQUAL(actual.maxWidth, expected.maxWidth) << "maxWidth mismatch";
    UT_ASSERT_EQUAL(actual.maxHeight, expected.maxHeight) << "maxHeight mismatch";
    UT_ASSERT_EQUAL(actual.frameWidth, expected.frameWidth) << "frameWidth mismatch";
    UT_ASSERT_EQUAL(actual.frameHeight, expected.frameHeight) << "frameHeight mismatch";
    UT_ASSERT_EQUAL(actual.colorDepth, expected.colorDepth) << "colorDepth mismatch";
    UT_ASSERT_EQUAL(actual.maxFrameRate, expected.maxFrameRate) << "maxFrameRate mismatch";
    UT_ASSERT_EQUAL(actual.supportsAlpha, expected.supportsAlpha) << "supportsAlpha mismatch";
    UT_ASSERT_EQUAL(actual.supportsZOrder, expected.supportsZOrder) << "supportsZOrder mismatch";
    UT_ASSERT_EQUAL(actual.vsyncDisplayLatency, expected.vsyncDisplayLatency) << "vsyncDisplayLatency mismatch";
}


/** @} */ // End of Plane_Control_Halif_Test_Util
/** @} */ // End of Plane_Control_Halif_Test
/** @} */ // End of Plane_Control
/** @} */ // End of HPK