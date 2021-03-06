﻿#pragma once
#include "xx_ref.h"
#include "xx_ptr.h"

namespace xx
{
	struct String;
	struct BBuffer;

	// 支持 MemPool 的类都应该从该基类派生
	struct Object
	{
		virtual ~Object() {}

		// 加持
		inline void AddRef()
		{
			assert(refCount() != 0xFFFFFFFF);		// 防止出现值类型被加持
			++refCount();
		}

		/*
		if (tsFlags())
		{
			str.Append("{ ... }");
			return;
		}
		else tsFlags() = 1;

		str.Append("{ \"type\" : \"SceneObj\"");
		ToStringCore(str);
		str.Append(" }");

		tsFlags() = 0;
		*/
		virtual void ToString(String &str) const;

		/*
		this->BaseType::ToStringCore(str);
		str.Append("{ \"field\" : ", field, " }");
		*/
		virtual void ToStringCore(String &str) const;

		// 取代 std::cout, 用起来方便一些. 实现在 xx_string.h
		template<typename ...Args>
		void Cout(Args const& ... args);

		/*
		this->BaseType::ToBBuffer(bb);
		bb.Write(.............);
		*/
		inline virtual void ToBBuffer(BBuffer &bb) const
		{
			assert(false);
		};

		/*
		if (auto rtv = this->BaseType::FromBBuffer(bb)) return rtv;
		return bb.Read(.............);
		*/
		inline virtual int FromBBuffer(BBuffer &bb)
		{
			assert(false);
			return 0;
		};

		// 减持 或 析构 + 回收变野( 代码的实现在 xx_mempoolbase.h 的尾部 )
		void Release() noexcept;

		/***********************************************************************************/
		// 下面是访问头部数据的各种 helpers
		/***********************************************************************************/

		inline MemHeader_Object& memHeader() { return *((MemHeader_Object*)this - 1); }
		inline MemHeader_Object& memHeader() const { return *((MemHeader_Object*)this - 1); }

		inline uint64_t const& versionNumber() const { return memHeader().versionNumber; }
		inline uint64_t pureVersionNumber() const { return versionNumber() & 0x00FFFFFFFFFFFFFFu; }

		inline uint32_t& refCount() { return memHeader().refCount; }
		inline uint32_t const& refCount() const { return memHeader().refCount; }

		inline uint16_t const& typeId() const { return memHeader().typeId; }

		inline uint16_t& tsFlags() { return memHeader().tsFlags; }
		inline uint16_t& tsFlags() const { return memHeader().tsFlags; }

		inline MemPool& mempool() const { return *(memHeader().mempool); }
		inline MemPool& mempool() { return *(memHeader().mempool); }
	};

	/***********************************************************************************/
	// type_traits
	/***********************************************************************************/

	// IsObjectPointer

	template<typename T>
	struct IsObjectPointer
	{
		static const bool value = false;
	};
	template<typename T>
	struct IsObjectPointer<T*>
	{
		static const bool value = !std::is_void<T>::value && std::is_base_of<Object, T>::value;
	};
	template<typename T>
	constexpr bool IsObjectPointer_v = IsObjectPointer<T>::value;


	// IsObjectStruct_v

	template<typename T>
	constexpr bool IsObjectStruct_v = std::is_base_of<Object, T>::value;


	// IsObject

	template<typename T>
	struct IsObject
	{
		static const bool value = !std::is_void<T>::value && std::is_base_of<Object, T>::value;
	};
	template<typename T>
	struct IsObject<T*>
	{
		static const bool value = !std::is_void<T>::value && std::is_base_of<Object, T>::value;
	};
	template<typename T>
	struct IsObject<Ref<T>>
	{
		static const bool value = true;
	};
	template<typename T>
	struct IsObject<Ptr<T>>
	{
		static const bool value = true;
	};
	template<typename T>
	struct IsObject<Dock<T>>
	{
		static const bool value = true;
	};
	template<typename T>
	constexpr bool IsObject_v = IsObject<T>::value;


	// 扫类型列表中是否含有 Object* 或 Ref<Object> 类型
	template<typename ...Types>
	struct ExistsObject
	{
		template<class Tuple, size_t N> struct TupleScaner {
			static constexpr bool Exec()
			{
				return IsObject_v< std::tuple_element_t<N - 1, Tuple> > ? true : TupleScaner<Tuple, N - 1>::Exec();
			}
		};
		template<class Tuple> struct TupleScaner<Tuple, 1> {
			static constexpr bool Exec()
			{
				return IsObject_v< std::tuple_element_t<0, Tuple> >;
			}
		};
		static constexpr bool value = TupleScaner<std::tuple<Types...>, sizeof...(Types)>::Exec();
	};

	// 生成 Object 的原始 typeId 映射
	template<> struct TypeId<Object> { static const uint16_t value = 1; };
}
