// File: mstream.h
// Author: PENGUINLIONG
#ifndef OCTOON_IO_MSTREAM_H_
#define OCTOON_IO_MSTREAM_H_

#include <octoon/io/iostream.h>
#include <octoon/io/membuf.h>

namespace octoon
{
	namespace io
	{
		class OCTOON_EXPORT imstream final : public istream
		{
		public:
			imstream() noexcept;
			imstream(std::size_t capacity, const ios_base::open_mode mode) noexcept;
			imstream(std::vector<std::uint8_t>&& buffer, const ios_base::open_mode mode) noexcept;
			imstream(const std::vector<std::uint8_t>& buffer, const ios_base::open_mode mode) noexcept;

			bool is_open() const noexcept;

			imstream& open(std::size_t capacity, const ios_base::open_mode mode) noexcept;
			imstream& open(std::vector<std::uint8_t>&& buffer, const ios_base::open_mode mode) noexcept;
			imstream& open(const std::vector<std::uint8_t>& buffer, const ios_base::open_mode mode) noexcept;

			imstream& close() noexcept;

		private:
			imstream(const imstream&) = delete;
			imstream& operator=(const imstream&) = delete;

		private:
			membuf buf_;
		};

		class OCTOON_EXPORT omstream final : public ostream
		{
		public:
			omstream() noexcept;
			omstream(std::size_t capacity, const ios_base::open_mode mode) noexcept;
			omstream(std::vector<std::uint8_t>&& buffer, const ios_base::open_mode mode) noexcept;
			omstream(const std::vector<std::uint8_t>& buffer, const ios_base::open_mode mode) noexcept;

			bool is_open() const noexcept;

			omstream& open(std::size_t capacity, const ios_base::open_mode mode) noexcept;
			omstream& open(std::vector<std::uint8_t>&& buffer, const ios_base::open_mode mode) noexcept;
			omstream& open(const std::vector<std::uint8_t>& buffer, const ios_base::open_mode mode) noexcept;

			omstream& close() noexcept;

		private:
			omstream(const omstream&) = delete;
			omstream& operator=(const omstream&) = delete;

		private:
			membuf buf_;
		};

		class OCTOON_EXPORT mstream final : public iostream
		{
		public:
			mstream() noexcept;
			mstream(std::size_t capacity, const ios_base::open_mode mode) noexcept;
			mstream(std::vector<std::uint8_t>&& buffer, const ios_base::open_mode mode) noexcept;
			mstream(const std::vector<std::uint8_t>& buffer, const ios_base::open_mode mode) noexcept;

			bool is_open() const noexcept;

			mstream& open(std::size_t capacity, const ios_base::open_mode mode) noexcept;
			mstream& open(std::vector<std::uint8_t>&& buffer, const ios_base::open_mode mode) noexcept;
			mstream& open(const std::vector<std::uint8_t>& buffer, const ios_base::open_mode mode) noexcept;

			mstream& close() noexcept;

		private:
			mstream(const mstream&) = delete;
			mstream& operator=(const mstream&) = delete;

		private:
			membuf buf_;
		};
	}
}

#endif
