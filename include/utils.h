#pragma once

#include "pch.h"

//#define HOOK_EXPORT __declspec(dllexport)
//#define EXTERN_HOOK_EXPORT extern "C" HOOK_EXPORT

#define HOOK_EXPORT extern "C"
#define EXTERN_HOOK_EXPORT HOOK_EXPORT

#ifdef _WIN64
#define HOOK_CALL_CONVENTION WINAPI
#else
#define HOOK_CALL_CONVENTION WINAPI
#endif

#define GLPROXY_NEED_OGL_TYPES
#ifdef GLPROXY_NEED_OGL_TYPES
typedef double         GLclampd;
typedef double         GLdouble;
typedef float          GLclampf;
typedef float          GLfloat;
typedef int            GLint;
typedef int            GLsizei;
typedef short          GLshort;
typedef signed char    GLbyte;
typedef unsigned char  GLboolean;
typedef unsigned char  GLubyte;
typedef unsigned int   GLbitfield;
typedef unsigned int   GLenum;
typedef unsigned int   GLuint;
typedef unsigned short GLushort;
typedef void GLvoid;
#endif // GLPROXY_NEED_OGL_TYPES

#define DEBUG_INFO3(...) \
	do \
	{ \
		auto log_header = fmt::format("{}:{}:{} ", __FILE__, __FUNCTION__, __LINE__); \
		auto other_info = fmt::format(__VA_ARGS__); \
		spdlog::info(log_header + other_info + "\n"); \
	} \
	while(0) \

#define CCAT(a, b) a##b
#define CAT(a, b) CCAT(a, b)

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)

#define UNIQUIFY2(x) CAT(x, __LINE__)
#define UNIQUIFY(x) UNIQUIFY2(x)

#define STRINGIFY_ENUM(x) \
case x: \
	return STRINGIFY(x) \

template<typename... Ts>
inline auto DEBUG_INFO_HELPER(Ts&&... ts)
{
	/*
	auto result = std::string{};
	if constexpr (sizeof...(ts) == 0)
	{
		result += fmt::format("{}", t);
	}
	else
	{
		if constexpr (std::is_pointer_v<T>)
		{
			result += fmt::format("{} ", reinterpret_cast<void*>(t)));
		}
		else
		{
			result += fmt::format("{} ", t);
		}
		result += DEBUG_INFO_HELPER(ts...);
	}
	return result;
	*/
	std::stringstream ss;
	((ss << std::forward<Ts>(ts) << " "), ...);
	return ss.str();
}

#ifndef NDEBUG

#define DEBUG(...) \
	do \
	{ \
		auto UNIQUIFY(log_header) = fmt::format("{}:{}:{} ", __FILE__, __FUNCTION__, __LINE__); \
		auto UNIQUIFY(other_info) = DEBUG_INFO_HELPER(##__VA_ARGS__); \
		spdlog::info(UNIQUIFY(log_header) + UNIQUIFY(other_info)); \
	} \
	while(0) \

#else

#define DEBUG(...) \

#endif

#define INFO(...) \
	do \
	{ \
		auto UNIQUIFY(log_header) = fmt::format("{}:{}:{} ", __FILE__, __FUNCTION__, __LINE__); \
		auto UNIQUIFY(other_info)= fmt::format(##__VA_ARGS__); \
		spdlog::info(UNIQUIFY(log_header) + UNIQUIFY(other_info)); \
	} \
	while(0) \

#define ERR(...) \
	do \
	{ \
		auto UNIQUIFY(log_header) = fmt::format("{}:{}:{} ", __FILE__, __FUNCTION__, __LINE__); \
		auto UNIQUIFY(other_info)= DEBUG_INFO_HELPER(##__VA_ARGS__); \
		spdlog::error(UNIQUIFY(log_header) + UNIQUIFY(other_info)); \
	} \
	while(0) \

#define RETURN_DUMMY() return 1 \

#define RETURN_DUMMY_PTR() return (const GLubyte*)1 \

#define PANIC(...) \
	do \
	{ \
		auto log_header = fmt::format("{}:{}:{} ", __FILE__, __FUNCTION__, __LINE__); \
		auto other_info = DEBUG_INFO_HELPER(__VA_ARGS__); \
		spdlog::error(log_header + other_info); \
		PanicMessageBox(log_header + other_info); \
		__debugbreak(); \
		exit(-1); \
	} \
	while(0) \

namespace RaysterizerEngine::Util
{
	//
	inline HMODULE GetCurrentModule()
	{
		HMODULE hModule = NULL;
		// hModule is NULL if GetModuleHandleEx fails.
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
						  | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
						  (LPCTSTR)&GetCurrentModule, &hModule);
		return hModule;
	}

	///////////////////////////////////////////////////////////////////
	// String
	///////////////////////////////////////////////////////////////////

	inline std::vector<std::string> SplitString(std::string str, std::string delimiter)
	{
		std::vector<std::string> result;
		auto delimiter_length = delimiter.length();
		std::size_t cur_pos = 0;
		std::size_t offset = 0;
		while ((offset = str.find(delimiter, cur_pos)) != std::string::npos)
		{
			result.push_back(str.substr(cur_pos, offset - cur_pos));
			offset += delimiter_length;
			cur_pos = offset;
		}
		if (cur_pos != str.length())
		{
			result.push_back(str.substr(cur_pos));
		}
		return result;
	}

	inline void ReplaceString(std::string& subject, std::string_view search, std::string_view replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	inline std::wstring ConvertToWString(std::string str)
	{
		return std::wstring(std::begin(str), std::end(str));
	}

	inline std::string ConvertToString(std::wstring str)
	{
		return std::string(std::begin(str), std::end(str));
	}

	///////////////////////////////////////////////////////////////////
	// LLVM errors
	///////////////////////////////////////////////////////////////////

	//LLVM error

#define StringErrorImpl(x) \
  llvm::createStringError("[" + std::string(__FILE__) + " " + std::string(__FUNCTION__) + ":" + std::to_string(__LINE__) + "] " + (x)) \

#define StringErrorImpl(...) \
  llvm::createStringError(fmt::format("[{} {}:{}] ", __FILE__, __FUNCTION__, __LINE__) + fmt::format(__VA_ARGS__))

#define StringError(...) \
  StringErrorImpl(__VA_ARGS__) \

//x has to be an llvm::Error
#define ReturnIfError(x) \
    do \
    { \
      if (auto err = x) \
      { \
        return std::move(err); \
      } \
    } \
    while(0) \

/*
#define AssignOrReturnError(variable, expected_call) \
  do \
  { \
    auto llvmExpectedResult = expected_call; \
    ReturnIfError(llvmExpectedResult.takeError()); \
    variable = std::move(*llvmExpectedResult); \
  } \
  while(0) \
*/

/*
#define AssignOrReturnError(variable, expected_call) \
	ReturnIfError(expected_call.takeError()); \
	variable = std::move(*expected_call); \
*/

#define AssignOrReturnError(variable, expected_call) \
	auto UNIQUIFY(llvmExpectedResult) = expected_call; \
	ReturnIfError(UNIQUIFY(llvmExpectedResult).takeError()); \
	variable = std::move(*(UNIQUIFY(llvmExpectedResult))); \

#define PanicMessageBox(x) \
  do \
  { \
    std::string output = "[" + std::string(__FILE__) + " " + std::string(__FUNCTION__) + ":" + std::to_string(__LINE__) + "] " + (x); \
	MessageBox(NULL, output.c_str(), "PANIC", MB_OK); \
	__debugbreak(); \
	system("pause"); \
  } \
  while (0) \

#include <system_error>

#define ReturnIfNotSucceeded(x) \
  do \
  { \
	HRESULT UNIQUIFY(hr) = x; \
	if (!SUCCEEDED(UNIQUIFY(hr))) \
	{ \
		return StringError(std::system_category().message(UNIQUIFY(hr))); \
	} \
  } \
  while (0) \

#define ReturnIfNotSucceededEx(x, error_message) \
  do \
  { \
	HRESULT UNIQUIFY(hr) = x; \
	if (!SUCCEEDED(UNIQUIFY(hr))) \
	{ \
		__debugbreak(); \
		return StringError(error_message + " " + std::system_category().message(UNIQUIFY(hr))); \
	} \
  } \
  while (0) \

#define PanicError(x) \
    do \
    { \
		std::stringstream ss; \
	    llvm::logAllUnhandledErrors(std::move(err), ss, "Error"); \
		PanicMessageBox(ss.str()); \
		__debugbreak(); \
    } \
    while(0) \

#define PanicIfError(x) \
    do \
    { \
      if (auto err = x) \
      { \
		  std::stringstream ss; \
	      llvm::logAllUnhandledErrors(std::move(err), ss, "Error"); \
		  PanicMessageBox(ss.str()); \
		  __debugbreak(); \
      } \
    } \
    while(0) \

	template<typename T>
	inline T AssignOrPanicIfExpectedError2(llvm::Expected<T> expected_call)
	{
		if (expected_call)
		{
			return std::move(*expected_call);
		}
		PanicIfError(expected_call.takeError());
		return std::move(*expected_call);
	}

#define AssignOrPanic(x) \
	RaysterizerEngine::Util::AssignOrPanicIfExpectedError2(x)

	template<typename T, typename = std::enable_if_t<std::is_pointer_v<T>>>
	inline llvm::Error InvalidPtrImpl(T t, std::string error_string = "")
	{
		if (IsBadReadPtr(t, sizeof(t)))
		{
			return StringError(error_string);
		}
	}

#define LogError(x) \
	do \
	{ \
		auto UNIQUIFY(log_header) = fmt::format("{}:{}:{} ", __FILE__, __FUNCTION__, __LINE__); \
		llvm::logAllUnhandledErrors(std::move(x), std::cerr, UNIQUIFY(log_header)); \
	} \
	while(0) \

#define InvalidPtr(x) \
  Util::InvalidPtrImpl((x), #x" is invalid") \

#define ReturnIfInvalidPtr(x) \
  do \
  { \
    if (auto err = InvalidPtr((x))) \
    { \
      return std::move(err); \
    } \
  } \
  while(0) \

	template<typename F>
	class OnceExecutor
	{
	public:
		template<typename T>
		OnceExecutor(T&& t) : f(std::forward<T>(t)) {}
	private:
		F f;
	};

	struct OnceExecutorHelper
	{
		template<typename F>
		OnceExecutor<F> operator+(F&& f) const { return std::forward<F>(f); }
	};

	class OnceHelper
	{
	public:
		template<typename Func>
		void operator+(Func&& func)
		{
			static std::once_flag once_flag;
			std::call_once(once_flag, func);
		}
	};

#define NoError() \
	llvm::ErrorSuccess() \

#define ConsumeError(...) \
	llvm::consumeError(std::move(__VA_ARGS__)) \

#define CallOnce RaysterizerEngine::Util::OnceHelper() + [&]()


	template<typename T, typename T2>
	constexpr T AlignUp(T val, T2 alignment)
	{
		auto mask = alignment - 1;
		return (val + mask) & ~mask;
	}

	inline void WaitUntilKeyPress()
	{
#ifdef WIN32
		system("pause");
#endif
	}

	inline void WaitUntilKeyPressAsync()
	{
#ifdef WIN32
		while (!(GetAsyncKeyState(VK_F6) & 0x8000))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
#endif
	}

	inline void Breakpoint()
	{
#ifdef WIN32
		__debugbreak();
#endif
	}

	// https://github.com/fenbf/StringViewTests/blob/master/StringViewTest.cpp#L150
	inline std::vector<std::string_view> SplitString(std::string_view str, std::string_view delims = " ")
	{
		std::vector<std::string_view> output;
		output.reserve(str.size() / 2);

		for (auto first = str.data(), second = str.data(), last = first + str.size(); second != last && first != last; first = second + 1)
		{
			second = std::find_first_of(first, last, std::cbegin(delims), std::cend(delims));

			if (first != second)
				output.emplace_back(first, second - first);
		}

		return output;
	}

	inline std::string JoinStrings(std::vector<std::string_view> strs, std::string_view delims = " ")
	{
		std::string out;
		auto expected_length = 0;
		for (const auto& s : strs)
		{
			expected_length += s.length() + delims.length();
		}
		out.reserve(expected_length);

		for (auto i = 0; i < strs.size(); i++)
		{
			const auto& s = strs[i];
			out += s;
			if (i != strs.size() - 1)
			{
				out += delims;
			}
		}

		return out;
	}

	inline std::string JoinStrings(std::vector<std::string> strs, std::string_view delims = " ")
	{
		std::vector<std::string_view> string_views;
		for (const auto& str : strs)
		{
			string_views.emplace_back(str);
		}
		return JoinStrings(string_views, delims);
	}

	template<typename T>
	inline std::vector<T> ReadFileAsVec(const fs::path& path)
	{
		std::ifstream file(path, std::ios::binary);

		if (file)
		{
			file.seekg(0, std::ios::end);
			auto file_size = file.tellg();
			file.seekg(0, std::ios::beg);

			if (file_size % sizeof(T) != 0)
			{
				PANIC("File size not equally split {} / {}", file_size, sizeof(T));
			}

			std::vector<T> data(file_size / sizeof(T));

			file.read(reinterpret_cast<char*>(data.data()), file_size);

			return data;
		}

		PANIC("NO DATA");

		return {};
	}
}

/////////////////////////////////////////////////////////////////////////////
/// Exception handling
/////////////////////////////////////////////////////////////////////////////

inline int SEHHandler(unsigned int code, struct _EXCEPTION_POINTERS*)
{
	auto GetExceptionDescription = [](const int code) -> std::string
	{
		switch (code)
		{
		case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
		case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
		case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
		case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
		case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
		case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
		case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
		case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
		case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
		case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
		case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
		case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
		default:
		{
			std::stringstream ss;
			ss << std::uppercase << std::hex << "UNKNOWN EXCEPTION (" << code << ")";
			return ss.str();
		}
		}
	};

	//TODO might throw another exception :(
	auto exception_description = std::string(GetExceptionDescription(code));
	//PANIC("Caught SEH exception: " + exception_description);

	return EXCEPTION_EXECUTE_HANDLER;
}

struct SEHExecutorHelper
{
	template<typename F>
	bool operator+(F&& f)
	{
		return [&]() -> bool
		{
			__try
			{
				[&]()
				{
					f();
				}();
			}
			__except (SEHHandler(GetExceptionCode(), GetExceptionInformation()))
			{
				ERR("Got an SEH exception\n");
				return true;
			}
			return false;
		}();
	}
};

#define SEHCall \
   SEHExecutorHelper() + [&]() \

#define CALL_WITH_SEH(...) \
do \
{ \
  [&]() \
  { \
    __try \
    { \
      [&]() \
      { \
        __VA_ARGS__ \
      }(); \
    } \
    __except (SEHHandler(GetExceptionCode(), GetExceptionInformation())) \
    { \
      printf("Got an SEH exception\n"); \
    } \
  }(); \
} \
while(0) \

#define CALL_WITH_SEH_RETURN(...) \
  [&]() -> bool \
  { \
    __try \
    { \
      [&]() \
      { \
        __VA_ARGS__ \
      }(); \
    } \
    __except (SEHHandler(GetExceptionCode(), GetExceptionInformation())) \
    { \
      return false; \
    } \
    return true; \
  }() \
