/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __UTIL_IMAGE_BUFFER_HPP__
#define __UTIL_IMAGE_BUFFER_HPP__

#include "util_image_definitions.hpp"
#include <mathutil/uvec.h>
#include <cinttypes>
#include <functional>
#include <optional>
#include <array>
#include <memory>

struct Color;
namespace uimg
{
	class DLLUIMG ImageBuffer
		: public std::enable_shared_from_this<ImageBuffer>
	{
	public:
		enum class Format : uint8_t
		{
			None = 0u,
			RGB8,
			RGBA8,
			RGB16,
			RGBA16,
			RGB32,
			RGBA32,
			Count,

			RGB_LDR = RGB8,
			RGBA_LDR = RGBA8,
			RGB_HDR = RGB16,
			RGBA_HDR = RGBA16,
			RGB_FLOAT = RGB32,
			RGBA_FLOAT = RGBA32
		};
		enum class Channel : uint8_t
		{
			Red = 0,
			Green,
			Blue,
			Alpha,

			Count,

			R = Red,
			G = Green,
			B = Blue,
			A = Alpha
		};
		enum class ToneMapping : uint8_t
		{
			GammaCorrection = 0,
			Reinhard,
			HejilRichard,
			Uncharted,
			Aces,
			GranTurismo
		};
		using Offset = size_t;
		using Size = size_t;
		using PixelIndex = uint32_t;
		using LDRValue = uint8_t;
		using HDRValue = uint16_t;
		using FloatValue = float;
		class PixelIterator;
		struct DLLUIMG PixelView
		{
			Offset GetOffset() const;
			PixelIndex GetPixelIndex() const;
			uint32_t GetX() const;
			uint32_t GetY() const;
			const void *GetPixelData() const;
			void *GetPixelData();
			LDRValue GetLDRValue(Channel channel) const;
			HDRValue GetHDRValue(Channel channel) const;
			FloatValue GetFloatValue(Channel channel) const;
			void SetValue(Channel channel,LDRValue value);
			void SetValue(Channel channel,HDRValue value);
			void SetValue(Channel channel,FloatValue value);
			void CopyValues(const PixelView &outOther);
			void CopyValue(Channel channel,const PixelView &outOther);

			ImageBuffer &GetImageBuffer() const;
		private:
			PixelView(ImageBuffer &imgBuffer,Offset offset);
			Offset GetAbsoluteOffset() const;
			friend PixelIterator;
			friend ImageBuffer;
			ImageBuffer &m_imageBuffer;
			Offset m_offset = 0u;
		};
		class DLLUIMG PixelIterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = PixelView;
			using difference_type = PixelView;
			using pointer = PixelView*;
			using reference = PixelView&;

			PixelIterator &operator++();
			PixelIterator operator++(int);
			PixelView &operator*();
			PixelView *operator->();
			bool operator==(const PixelIterator &other) const;
			bool operator!=(const PixelIterator &other) const;
		private:
			friend ImageBuffer;
			PixelIterator(ImageBuffer &imgBuffer,Offset offset);
			PixelView m_pixelView;
		};

		static std::shared_ptr<ImageBuffer> Create(void *data,uint32_t width,uint32_t height,Format format,bool ownedExternally=true);
		static std::shared_ptr<ImageBuffer> CreateWithCustomDeleter(void *data,uint32_t width,uint32_t height,Format format,const std::function<void(void*)> &customDeleter);
		static std::shared_ptr<ImageBuffer> Create(const void *data,uint32_t width,uint32_t height,Format format);
		static std::shared_ptr<ImageBuffer> Create(uint32_t width,uint32_t height,Format format);
		static std::shared_ptr<ImageBuffer> Create(ImageBuffer &parent,uint32_t x,uint32_t y,uint32_t w,uint32_t h);
		// Order: Right, left, up, down, forward, backward
		static std::shared_ptr<ImageBuffer> CreateCubemap(const std::array<std::shared_ptr<ImageBuffer>,6> &cubemapSides);
		static Size GetPixelSize(Format format);
		static uint8_t GetChannelSize(Format format);
		static uint8_t GetChannelCount(Format format);
		static LDRValue ToLDRValue(HDRValue value);
		static LDRValue ToLDRValue(FloatValue value);
		static HDRValue ToHDRValue(LDRValue value);
		static HDRValue ToHDRValue(FloatValue value);
		static FloatValue ToFloatValue(LDRValue value);
		static FloatValue ToFloatValue(HDRValue value);
		static Format ToLDRFormat(Format format);
		static Format ToHDRFormat(Format format);
		static Format ToFloatFormat(Format format);
		static Format ToRGBFormat(Format format);
		static Format ToRGBAFormat(Format format);

		ImageBuffer(const ImageBuffer&)=default;
		ImageBuffer &operator=(const ImageBuffer&)=default;
		Format GetFormat() const;
		uint32_t GetWidth() const;
		uint32_t GetHeight() const;
		uint8_t GetChannelCount() const;
		uint8_t GetChannelSize() const;
		Size GetPixelSize() const;
		uint32_t GetPixelCount() const;
		bool HasAlphaChannel() const;
		bool IsLDRFormat() const;
		bool IsHDRFormat() const;
		bool IsFloatFormat() const;
		const void *GetData() const;
		void *GetData();
		std::shared_ptr<ImageBuffer> Copy() const;
		std::shared_ptr<ImageBuffer> Copy(Format format) const;
		void Copy(ImageBuffer &dst,uint32_t xSrc,uint32_t ySrc,uint32_t xDst,uint32_t yDst,uint32_t w,uint32_t h) const;
		void Convert(Format targetFormat);
		void SwapChannels(Channel channel0,Channel channel1);
		void ToLDR();
		void ToHDR();
		void ToFloat();
		std::shared_ptr<ImageBuffer> ApplyToneMapping(ToneMapping toneMappingMethod);
		std::shared_ptr<ImageBuffer> ApplyToneMapping(const std::function<std::array<uint8_t,3>(const Vector3&)> &fToneMapper);

		Size GetSize() const;

		void Clear(const Color &color);
		void Clear(const Vector4 &color);
		void ClearAlpha(LDRValue alpha=std::numeric_limits<LDRValue>::max());

		PixelIndex GetPixelIndex(uint32_t x,uint32_t y) const;
		Offset GetPixelOffset(uint32_t x,uint32_t y) const;
		Offset GetPixelOffset(PixelIndex index) const;

		void Read(Offset offset,Size size,void *outData);
		void Write(Offset offset,Size size,const void *inData);
		void Resize(Size width,Size height);

		void FlipHorizontally();
		void FlipVertically();

		void InitPixelView(uint32_t x,uint32_t y,PixelView &pxView);
		PixelView GetPixelView(Offset offset=0);
		PixelView GetPixelView(uint32_t x,uint32_t y);

		void SetPixelColor(uint32_t x,uint32_t y,const std::array<uint8_t,4> &color);
		void SetPixelColor(PixelIndex index,const std::array<uint8_t,4> &color);
		void SetPixelColor(uint32_t x,uint32_t y,const std::array<uint16_t,4> &color);
		void SetPixelColor(PixelIndex index,const std::array<uint16_t,4> &color);
		void SetPixelColor(uint32_t x,uint32_t y,const Vector4 &color);
		void SetPixelColor(PixelIndex index,const Vector4 &color);

		ImageBuffer *GetParent();
		const std::pair<uint64_t,uint64_t> &GetPixelCoordinatesRelativeToParent() const;
		Offset GetAbsoluteOffset(Offset localOffset) const;

		PixelIterator begin();
		PixelIterator end();
	private:
		static void Convert(ImageBuffer &srcImg,ImageBuffer &dstImg,Format targetFormat);
		ImageBuffer(const std::shared_ptr<void> &data,uint32_t width,uint32_t height,Format format);
		std::pair<uint32_t,uint32_t> GetPixelCoordinates(Offset offset) const;
		void Reallocate();
		std::shared_ptr<void> m_data = nullptr;
		uint32_t m_width = 0u;
		uint32_t m_height = 0u;
		Format m_format = Format::None;

		std::weak_ptr<ImageBuffer> m_parent = {};
		std::pair<uint64_t,uint64_t> m_offsetRelToParent = {};
	};

	DLLUIMG std::optional<ImageBuffer::ToneMapping> string_to_tone_mapping(const std::string &str);
};

#endif
