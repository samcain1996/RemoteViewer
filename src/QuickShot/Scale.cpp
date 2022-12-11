#include "Scale.h"


/* ----- Pixel Map ----- */

size_t CoordinateToIndex(const Resolution& res, const Coordinate& coord) {
    return coord.second * res.width + coord.first;
}

Coordinate IndexToCoordinate(const Resolution& res, const size_t index) {
    return { index % res.width , index / res.width };
}

size_t ConvertIndex(const size_t index, const bool toAbsoluteIndex) {
    return toAbsoluteIndex ? index * BYTES_PER_PIXEL : index / BYTES_PER_PIXEL;
}

Pixel GetPixel(PixelData& data, const size_t index, const bool isAbsoluteIndex) {
    const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index);
    return Pixel{ data }.subspan(idx, BYTES_PER_PIXEL);
}

ConstPixel GetPixel(const PixelData& data, const size_t index, const bool isAbsoluteIndex) {
    const size_t idx = isAbsoluteIndex ? index : ConvertIndex(index);
    return ConstPixel{ data }.subspan(idx, BYTES_PER_PIXEL);
}

void AssignPixel(Pixel& assignee, const ConstPixel& other) {
    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
        assignee[channel] = other[channel];
    }
}

Thing SubtractPixel(const ConstPixel& subFrom, const ConstPixel& sub) {
    Thing t;
    for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
        t[channel] = subFrom[channel] - sub[channel];
    }
    return t;
}

Neighbors FindDerivatives(const bool xDir, const Resolution& res, const PixelData& data, const Neighbors& neighbors) {

    static const Thing EmptyPixel = { 0, 0, 0, 0 };

    Neighbors derivatives{};

    int index = 0;

    std::for_each(neighbors.begin(), neighbors.end(), [&](const auto& pixelAndPos) {

        const auto& [pixelX, pixelY] = pixelAndPos.second;
        const int X_MAX = res.width - 1;
        const int Y_MAX = res.height - 1;

        const int MAX = xDir ? X_MAX : Y_MAX;
        const int axis = xDir ? pixelX : pixelY;

        const int xChange = xDir ? 1 : 0;
        const int yChange = xDir ? 0 : 1;

        const ConstPixel nextPixel = (axis >= MAX) ? EmptyPixel : GetPixel(data, CoordinateToIndex(res, { pixelX + xChange, pixelY + yChange }));
        const ConstPixel prevPixel = (axis <= 0) ? EmptyPixel : GetPixel(data, CoordinateToIndex(res, { pixelX - xChange, pixelY - yChange }));

        derivatives[index++] = { SubtractPixel(nextPixel, prevPixel), pixelAndPos.second };

        });

    return derivatives;

}

/* --------------------- */

/* ----- Scaler ----- */

PixelData Scaler::Scale(const PixelData& sourceImage,
    const Resolution& sourceResolution, const Resolution& destResolution) {

    // If the resolutions are the same, don't scale
    if (sourceResolution == destResolution) [[unlikely]] {
        return sourceImage;
    }

        // Scale based upon the scale method
        switch (method) {
        case ScaleMethod::NearestNeighbor:
            return NearestNeighbor(sourceImage, sourceResolution, destResolution);
        case ScaleMethod::Bilinear:
            return Bilinear(sourceImage, sourceResolution, destResolution);
        case ScaleMethod::Bicubic:
            return Bicubic(sourceImage, sourceResolution, destResolution);
        case ScaleMethod::Lanczos:
            return Lanczos(sourceImage, sourceResolution, destResolution);
        default:
            return PixelData();
        }

}

PixelData Scaler::Scale(const PixelData& sourceImage,
    const Resolution& sourceResolution, const ScaleRatio& scaleRatio) {

    return Scale(sourceImage, sourceResolution,
        Resolution{ sourceResolution.width * scaleRatio.xRatio, sourceResolution.height * scaleRatio.yRatio });
}

PixelData Scaler::Scale(const PixelData& sourceImage, const Resolution& sourceResolution, const Uint32 scalingFactor) {
    return Scale(sourceImage, sourceResolution, ScaleRatio(scalingFactor, scalingFactor));
}

// Get the ratio in the x and y directions between dest and source images
ScaleRatio Scaler::GetScaleRatio(const Resolution& source, const Resolution& dest) {
    return { (dest.width / (double)source.width), (dest.height / (double)source.height) };
}

Neighbors Scaler::GetNeighbors(const double x, const double y, const PixelData& source,
    const Resolution& src) {

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;

    const int x_l = floor(x);
    const int x_h = std::min((int)ceil(x), X_MAX_SRC);

    const int y_l = floor(y);
    const int y_h = std::min((int)ceil(y), Y_MAX_SRC);

    return
    {
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_l, y_l }), false), { x_l, y_l } },
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_h, y_l }), false), { x_h, y_l } },
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_l, y_h }), false), { x_l, y_h } },
        PixelAndPos{ GetPixel(source, CoordinateToIndex(src, { x_h, y_h }), false), { x_h, y_h } },
    };

}

// Upscale using nearest neighbor technique
PixelData Scaler::NearestNeighbor(const PixelData& source, const Resolution& src, const Resolution& dest) {

    PixelData scaled(CalculateBMPFileSize(dest));
    const auto [scaleX, scaleY] = GetScaleRatio(src, dest);

    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BYTES_PER_PIXEL) {

        // Convert pixel index to x,y coordinates
        const auto [destX, destY] = IndexToCoordinate(dest, ConvertIndex(absIndex, false));
        Pixel scaledPixel = GetPixel(scaled, absIndex);

        // Scale the coordinates
        const Coordinate mappedCoord = { destX / scaleX, destY / scaleY };

        // Convert the coordinates to index
        const size_t indexToMap = CoordinateToIndex(src, mappedCoord);
        ConstPixel sourcePixel = GetPixel(source, indexToMap, false);

        // Set scaledPixel equal to corresponding sourcePixel 
        AssignPixel(scaledPixel, sourcePixel);
    }

    return scaled;
}


PixelData Scaler::Bilinear(const PixelData& source, const Resolution& src, const Resolution& dest) {

    using enum Neighbor;

    // Init new image and get scale between new and source
    PixelData scaled(CalculateBMPFileSize(dest));
    const auto [destX, destY] = GetScaleRatio(src, dest);

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;

    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BYTES_PER_PIXEL) {

        // Get the pixel's X and Y coordinates
        const auto [scaledX, scaledY] = IndexToCoordinate(dest, ConvertIndex(absIndex, false));
        const double x = scaledX / destX;
        const double y = scaledY / destY;

        // Create new pixel from neighboring 4 pixels
        Pixel scaledPixel = GetPixel(scaled, absIndex);
        Neighbors neighbors = GetNeighbors(x, y, source, src);

        const int x_l = floor(x);
        const int y_l = std::min((int)ceil(y), Y_MAX_SRC);

        const double xl_weight = 1 - (x - x_l) / X_MAX_SRC;
        const double xh_weight = 1 - xl_weight;

        const double yl_weight = 1 - (y - y_l) / Y_MAX_SRC;
        const double yh_weight = 1 - yl_weight;

        for (size_t channel = 0; channel < NUM_COLOR_CHANNELS; ++channel) {
            scaledPixel[channel] =
                (xl_weight * neighbors[(int)TopLeft].first[channel] + xh_weight * neighbors[(int)TopRight].first[channel]) * yl_weight +
                (xl_weight * neighbors[(int)BottomLeft].first[channel] + xh_weight * neighbors[(int)BottomRight].first[channel]) * yh_weight;
        }

    }

    return scaled;
}

PixelData Scaler::Bicubic(const PixelData& source, const Resolution& src, const Resolution& dest) {

    using enum Neighbor;
    using Derivatives = Neighbors;
    using MatrixD = Matrix<double>;
    using Coefficients = std::array<MatrixD, BYTES_PER_PIXEL>;

    // Init new image and get scale between new and source
    PixelData scaled(CalculateBMPFileSize(dest));
    const auto [destX, destY] = GetScaleRatio(src, dest);

    const int X_MAX_SRC = src.width - 1;
    const int Y_MAX_SRC = src.height - 1;

    static const MatrixD multMat1{
        { 1, 0, 0, 0 },
        { 0, 0, 1, 0 },
        { -3, 3, -2, -1 },
        { 2, -2, 1, 1 }
    };

    static const MatrixD multMat2{
        { 1, 0, -3, 2 },
        { 0, 0, 3, -2 },
        { 0, 1, -2, 1 },
        { 0, 0, -1, 1 }
    };

    for (size_t absIndex = 0; absIndex < scaled.size(); absIndex += BYTES_PER_PIXEL) {

        const size_t pixelIndex = ConvertIndex(absIndex, false);

        const auto [scaledX, scaledY] = IndexToCoordinate(dest, pixelIndex);
        const double x = scaledX / destX;
        const double y = scaledY / destY;

        Neighbors neighbors = GetNeighbors(x, y, source, src);

        // Calculate derivatives
        Derivatives xDerivs = FindDerivatives(true, src, source, neighbors);
        Derivatives yDerivs = FindDerivatives(false, src, source, neighbors);
        Derivatives xyDerivs = FindDerivatives(true, src, source, yDerivs);

        const MatrixFunc functionMatrix = [&](const int index) {
            return MatrixD{
                { (double)neighbors[(int)TopLeft].first[index],  (double)neighbors[(int)BottomLeft].first[index],  (double)yDerivs[(int)TopLeft].first[index],    (double)yDerivs[(int)BottomLeft].first[index]},
                { (double)neighbors[(int)TopRight].first[index], (double)neighbors[(int)BottomRight].first[index], (double)yDerivs[(int)TopRight].first[index],   (double)yDerivs[(int)BottomRight].first[index]},
                { (double)xDerivs[(int)TopLeft].first[index],    (double)xDerivs[(int)BottomLeft].first[index],    (double)xyDerivs[(int)TopLeft].first[index],   (double)xyDerivs[(int)BottomLeft].first[index]},
                { (double)xDerivs[(int)TopRight].first[index],   (double)xDerivs[(int)BottomRight].first[index],   (double)xyDerivs[(int)TopRight].first[index],  (double)xyDerivs[(int)BottomRight].first[index]},
            };
        };

        Coefficients coefficients;
        for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
            coefficients[channel] = multMat1 * functionMatrix(channel) * multMat2;
        }

        const double xNormal = scaledX / (dest.width - 1);
        const double yNormal = scaledY / (dest.height - 1);
        const RowMatrix<double> xVec{ 1, xNormal, pow(xNormal, 2), pow(xNormal, 3) };
        const ColMatrix<double> yVec{ 1, yNormal, pow(yNormal, 2), pow(yNormal, 3) };

        Pixel scaledPixel = GetPixel(scaled, absIndex);

        for (size_t channel = 0; channel < BYTES_PER_PIXEL; ++channel) {
            scaledPixel[channel] = (int)(xVec * coefficients[channel] * yVec);
        }

    }

    return scaled;
}


// TODO: Implement Lanczos scaling
PixelData Scaler::Lanczos(const PixelData& source, const Resolution& src, const Resolution& dest) {
    return PixelData();
}

/* ------------------ */