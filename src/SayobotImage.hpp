#pragma once
#ifndef SAYOBOT_IMAGE_HPP
#define SAYOBOT_IMAGE_HPP

#define IM_PATH "C:\\Program Files (x86)\\ImageMagick-7.0.9-Q16-HDRI"

#include <Magick++.h>
#include <random>

namespace Sayobot
{
    struct TextStyle
    {
        TextStyle(
            const std::string& _color = "black", const double _pointsize = 12.0f,
            const std::string& _font_family = "monospace",
            const MagickCore::GravityType _gravity =
                MagickCore::GravityType::UndefinedGravity,
            const MagickCore::AlignType _align = MagickCore::AlignType::UndefinedAlign)
        {
            color = _color;
            pointsize = _pointsize;
            font_family = _font_family;
            gravity = _gravity;
            align = _align;
        }
        std::string color;
        double pointsize;
        std::string font_family;
        MagickCore::GravityType gravity;
        MagickCore::AlignType align;
    };

    class Image
    {
    public:
        Image()
        {
        }

        void Create(const size_t &width, const size_t &height)
        {
            this->image.size(Magick::Geometry(width, height));
            //this->image.read("xc:#FFFFFF");
        }

        void ReadFromFile(const std::string& path)
        {
            this->image.read(path);
        }

        void ReadFromUrl(const std::string& url)
        {
            this->image = Magick::Image(url);
        }

        void Crop(const Magick::Geometry& geometry)
        {
            this->image.crop(geometry);
        }

        void Crop(const size_t width, const size_t height, const size_t x_offset,
                  const size_t y_offset)
        {
            this->image.crop(Magick::Geometry(width, height, x_offset, y_offset));
        }

        void Rotate(const double degrees)
        {
            this->image.rotate(degrees);
        }

        void Drawtext(const std::string& str, const TextStyle& textStyle,
                      double x_offset, double y_offset)
        {
            Magick::DrawableList drawableList;
            drawableList.push_back(Magick::DrawableFillColor(textStyle.color));
            std::string ext =
                textStyle.font_family.substr(textStyle.font_family.find_last_of('.'));
            drawableList.push_back(Magick::DrawableFont(textStyle.font_family));
            drawableList.push_back(Magick::DrawablePointSize(textStyle.pointsize));
            drawableList.push_back(Magick::DrawableText(x_offset, y_offset, str));
            drawableList.push_back(Magick::DrawableGravity(textStyle.gravity));
            drawableList.push_back(Magick::DrawableTextAlignment(textStyle.align));
            this->image.draw(drawableList);
        }
        /*
         在图上绘制文字
         * 参数列表:
         *** str (const std::string&) 绘制的文字
         *** x_offset (double) 相对于起始点 (0, 0) 的x坐标偏移量
         *** y_offset (double) 相对于起始点 (0, 0) 的y坐标偏移量
         *** Color (const std::string&) 绘制的颜色 ( red 或者 #FF0000 都是可行的)
         *** size (double) 字体大小
         *** fontFamily (const std::string&)
             字体名或字体文件路径（如果是TTF格式的字体文件，最前面需要加@）
         *** 可选 opacity (double) 不透明度
         *** 可选 gravity 字体重力对齐方式

         ***** 以下TTF字体不可使用
         *** 可选 align 字体左右对齐方式
         *** 可选 stretch 字体扩张方式
         *** 可选 weight 字体粗细值，400为标准
        */
        void Drawtext(
            const std::string& str, double x_offset, double y_offset,
            const std::string& Color, double size, const std::string& fontFamily,
            const MagickCore::GravityType gravity =
                MagickCore::GravityType::UndefinedGravity,
            const MagickCore::AlignType align = MagickCore::AlignType::UndefinedAlign)
        {
            Magick::DrawableList drawableList;
            drawableList.push_back(Magick::DrawableFillColor(Color));
            drawableList.push_back(Magick::DrawableTextAlignment(align));
            drawableList.push_back(Magick::DrawableFont(fontFamily));
            drawableList.push_back(Magick::DrawablePointSize(size));
            drawableList.push_back(Magick::DrawableText(x_offset, y_offset, str));
            drawableList.push_back(Magick::DrawableGravity(gravity));
            this->image.draw(drawableList);
        }

        /*
         * 在图上贴画上另一张图
         * 参数列表:
         *** image (const Image&) 另一张图
         *** x_offset (size_t) 相对于起始点 (0, 0) 的x坐标偏移量
         *** y_offset (size_t) 相对于起始点 (0, 0) 的y坐标偏移量
         *** 可选 width 重新调整图片宽度
         *** 可选 height 重新调整图片高度
         */
        void DrawPic(Image& image, const size_t x_offset, const size_t y_offset,
                     size_t width = 0, size_t height = 0)
        {
            if (width && height)
                image.resize(Magick::Geometry(width, height));
            this->image.composite(
                image.image, x_offset, y_offset, MagickCore::OverCompositeOp);
        }

        /*
         * 在图上贴画上另一张图
         * 参数列表:
         *** path (const std::string&) 另一张图的路径
         *** x_offset (size_t) 相对于起始点 (0, 0) 的x坐标偏移量
         *** y_offset (size_t) 相对于起始点 (0, 0) 的y坐标偏移量
         *** 可选 width 重新调整图片宽度
         *** 可选 height 重新调整图片高度
         */
        void DrawPic(const std::string& path, size_t x_offset, size_t y_offset,
                     size_t width = 0, size_t height = 0)
        {
            Magick::Image newImage;
            newImage.read(path);

            if (width && height)
                newImage.resize(Magick::Geometry(width, height));
            this->image.composite(
                newImage, x_offset, y_offset, MagickCore::OverCompositeOp);
        }

        std::string GetRandomHash(int length = 16)
        {
            std::default_random_engine random(time(NULL));
            std::uniform_int_distribution<int> dist(0, 128);
            int randint = dist(random);
            return std::string(this->image.perceptualHash()).substr(randint, length);
        }

        std::string GetFullHash()
        {
            return std::string(this->image.perceptualHash());
        }
        /*
         * 保存图片
         */
        void Save(const std::string& path)
        {
            this->image.quality(100);
            this->image.write(path);
        }

        void resize(const Magick::Geometry& geometry)
        {
            this->image.resize(geometry);
        }

        void resize(size_t width, size_t height)
        {
            this->image.resize(Magick::Geometry(width, height));
        }

        // 汉明距离：
        // 比较两张图片的感知哈希值，计算出不同位的数量
        // 9% 以下 认为是相似
        static double HanmingDistance(const std::string& pHash1,
                                      const std::string& pHash2)
        {
            if (pHash1.size() != pHash2.size())
                return -1;
            int difference = 0;
            for (int i = 0; i < pHash1.size(); ++i)
            {
                if (pHash1[i] != pHash2[i])
                    ++difference;
            }
            return difference * 100.0 / pHash1.size();
        }

    private:
        Magick::Image image;
    };
} // namespace Sayobot

#endif // !SAYOBOT_IMAGE_HPP
