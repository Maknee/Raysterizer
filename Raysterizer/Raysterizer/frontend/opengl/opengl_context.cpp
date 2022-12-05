#include "opengl_context.h"

#define USE_NEW_SYNC_UBO_METHOD 1

namespace Raysterizer
{
	namespace OpenGL
	{
		static std::atomic<bool> init_raysterizer = false;

		namespace
		{
			static bool enable_message_callback = false;

			void EnableOpenGLDebugOutput()
			{
				enable_message_callback = true;
			}

			void DisableOpenGLDebugOutput()
			{
				enable_message_callback = false;
			}

			void GLAPIENTRY OpenGLMessageCallback(GLenum source,
					GLenum type,
					GLuint id,
					GLenum severity,
					GLsizei length,
					const GLchar* message,
					const void* userParam)
			{
				if (enable_message_callback)
				{
					auto is_error_message = type == GL_DEBUG_TYPE_ERROR ? "ERROR" : "DEBUG";
					auto message_string = std::string_view(message);
					auto error_message = fmt::format("OpenGLMessageCallback: {} | Type: {:08x} Severity: {} Message: {}", is_error_message, type, severity, message_string);
					DEBUG(error_message);
					if (type == GL_DEBUG_TYPE_ERROR)
					{
						for (const auto& program : disable_opengl_callback_check_for_programs)
						{
							if (GetModuleHandle(program.c_str()))
							{
								return;
							}
						}
						PANIC("OPENGL ERROR {}", error_message);
					}
				}
			}

			void SetupOpenGLDebugOutput()
			{
				glEnable(GL_DEBUG_OUTPUT);

				typedef void (APIENTRY* DEBUGPROC)(GLenum source,
					GLenum type,
					GLuint id,
					GLenum severity,
					GLsizei length,
					const GLchar* message,
					const void* userParam);
				auto& hook_manager = Raysterizer::Hooks::HookManager::Get();
				static const auto glDebugMessageCallback = (void(*)(DEBUGPROC, const void*))AssignOrPanic(hook_manager.GetOriginalFunctionByName("glDebugMessageCallback"));

				glDebugMessageCallback(&OpenGLMessageCallback, 0);
				EnableOpenGLDebugOutput();
			}

			void InitializeRaysterizer()
			{
				if (!init_raysterizer)
				{
					init_raysterizer = true;

					CallOnce
					{
						//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
					};

					vulkan_window = std::make_shared<GLFWWindow>(Config["window_name"], Config["width"], Config["height"]);
					GLFWwindow* glfw_window = (GLFWwindow*)vulkan_window->GetUnderlyingWindow();

					PanicIfError(c.Setup(vulkan_window));

					//SetupOpenGLDebugOutput();

					auto& context = Context::Get();
					auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
					raysterizer_vulkan_state.Setup();

					auto& state = context.state;

					GLint view_port[4] = { 0 };
					glGetIntegerv(GL_VIEWPORT, view_port);

					state.SetViewport(view_port[0], view_port[1], view_port[2], view_port[3]);

					// Some console stuff
					// https://stackoverflow.com/questions/191842/how-do-i-get-console-output-in-c-with-a-windows-program
					if (0)
					{
						FILE* fDummy;
						freopen_s(&fDummy, "CONIN$", "r", stdin);
						freopen_s(&fDummy, "CONOUT$", "w", stderr);
						freopen_s(&fDummy, "CONOUT$", "w", stdout);
					}
					if(0)
					{
						//Create a console for this application
						AllocConsole();

						// Get STDOUT handle
						HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
						int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
						FILE* COutputHandle = _fdopen(SystemOutput, "w");

						// Get STDERR handle
						HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
						int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
						FILE* CErrorHandle = _fdopen(SystemError, "w");

						// Get STDIN handle
						HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
						int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
						FILE* CInputHandle = _fdopen(SystemInput, "r");

						//make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
						std::ios::sync_with_stdio(true);

						// Redirect the CRT standard input, output, and error handles to the console
						freopen_s(&CInputHandle, "CONIN$", "r", stdin);
						freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
						freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

						// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
						// point to console as well
						std::ios::sync_with_stdio();


						//Clear the error state for each of the C++ standard stream objects. We need to do this, as
						//attempts to access the standard streams before they refer to a valid target will cause the
						//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
						//to always occur during startup regardless of whether anything has been read from or written to
						//the console or not.
						std::wcout.clear();
						std::cout.clear();
						std::wcerr.clear();
						std::cerr.clear();
						std::wcin.clear();
						std::cin.clear();
					}

					// Setup default frame buffer
					auto& frame_buffer_manager = context.frame_buffer_manager;
					auto fbo_id = DEFAULT_FRAME_BUFFER_OBJECT_ID;
					
					PanicIfError(frame_buffer_manager.AllocateFrameBufferObject(fbo_id));
					auto fbo = AssignOrPanic(frame_buffer_manager.GetFrameBufferObject(fbo_id));

					PanicIfError(state.SetActiveFrameBufferObjectId(fbo_id));
					
					auto& pipeline_manager = context.GetPipelineManager();
					PanicIfError(pipeline_manager.CreateGBufferPass(fbo_id));
					PanicIfError(pipeline_manager.SetActiveGBufferPass(fbo_id));
				}
			}

			void DrawFrameRaysterizer()
			{
				auto& pipeline_manager = Raysterizer::MiddleWare::RaysterizerVulkanState::Get().GetPipelineManager();

				auto current_hdc = wglGetCurrentDC();
				auto current_context = wglGetCurrentContext();

				//vulkan_window->Update();
				auto* window = static_cast<GLFWwindow*>(vulkan_window->GetUnderlyingWindow());
				glfwMakeContextCurrent(window);
				if (!glfwWindowShouldClose(window))
				{
					if (Config["glfw"]["poll_events"])
					{
						glfwPollEvents();
					}
					pipeline_manager.Draw();
					
					/*
					CALL_WITH_SEH
					(
						{
							pipeline_manager.Draw();
						}
					);
					*/
					glfwSwapBuffers(window);
					pipeline_manager.BeginFrame();
				}

				wglMakeCurrent(current_hdc, current_context);
			}
		}

		namespace
		{
			// Hook CallOriginalOpenGL itself... mostly used to disable opengl calls and make proxy calls
			flat_hash_map<void*, void*> proxy_hooked_function_calls;

			template<typename Func>
			inline Func GetOriginalOpenGLActual(Func hooked_function)
			{
				const auto original_function = Raysterizer::Hooks::GetFunction(hooked_function);
				return original_function;
			}

			template<typename Func>
			inline Func GetOriginalOpenGL(Func hooked_function)
			{
				static bool opengl_render = Config["raysterizer"]["opengl"]["render"];
				if (!opengl_render)
				{
					if (auto found = proxy_hooked_function_calls.find((void*)hooked_function); found != std::end(proxy_hooked_function_calls))
					{
						return (Func)found->second;
					}
				}

				return GetOriginalOpenGLActual(hooked_function);
			}

			template<typename Func, typename... Args>
			inline std::result_of_t<Func(Args...)> CallOriginalOpenGLOld(Func hooked_function, Args&&... args)
			{
				//using ReturnType = typename std::result_of_t<Func(Args...)>;
				//static const auto original_function = Raysterizer::Hooks::GetFunction(hooked_function);
				//static auto original_function = OriginalFunction<Func, Args...>{ Raysterizer::Hooks::GetFunction(hooked_function) };

				const auto original_function = GetOriginalOpenGL(hooked_function);
				return original_function(std::forward<Args>(args)...);
			}

			template<std::size_t UniqueIndex, typename Func, typename... Args>
			inline std::result_of_t<Func(Args...)> CallOriginalOpenGLActualUnique(Func hooked_function, Args&&... args)
			{
				static const auto original_function = GetOriginalOpenGLActual(hooked_function);
				return original_function(std::forward<Args>(args)...);
			};

			template<std::size_t UniqueIndex, typename Func, typename... Args>
			inline std::result_of_t<Func(Args...)> CallOriginalOpenGLUnique(Func hooked_function, Args&&... args)
			{
				static const auto original_function = GetOriginalOpenGL(hooked_function);
				return original_function(std::forward<Args>(args)...);
			};

			// https://stackoverflow.com/questions/15858141/conveniently-declaring-compile-time-strings-in-c/15863804#15863804
			template <char... Cs>
			struct ConstexprString
			{
				static constexpr int size = sizeof...(Cs);
				static constexpr char buffer[size] = { Cs... };
			};

			template <char... C1, char... C2>
			constexpr bool operator==(const ConstexprString<C1...>& lhs, const ConstexprString<C2...>& rhs)
			{
				if (lhs.size != rhs.size)
					return false;

				return std::is_same_v<std::integer_sequence<char, C1...>, std::integer_sequence<char, C2...>>;
			}

			template <typename F, std::size_t... Is>
			constexpr auto ConstexprStringBuilder(F f, std::index_sequence<Is...>)
			{
				return ConstexprString<f(Is)...>{};
			}

#define CONSTEXPR_STRING( x )                                              \
  ConstexprStringBuilder( []( std::size_t i ) constexpr { return x[i]; },  \
                 std::make_index_sequence<sizeof(x)>{} )

			////////////////

			template<std::size_t UniqueIndex, char... chars, typename Func, typename... Args>
			inline std::result_of_t<Func(Args...)> CallOriginalOpenGLActualUniqueWithName(ConstexprString<chars...> s, Func hooked_function, Args&&... args)
			{
				ScopedCPUProfileOpenGL(s.buffer);
				return CallOriginalOpenGLActualUnique<UniqueIndex>(hooked_function, std::forward<Args>(args)...);
			};

			template<std::size_t UniqueIndex, char... chars, typename Func, typename... Args>
			inline std::result_of_t<Func(Args...)> CallOriginalOpenGLUniqueWithName(ConstexprString<chars...> s, Func hooked_function, Args&&... args)
			{
				ScopedCPUProfileOpenGL(s.buffer);
				return CallOriginalOpenGLUnique<UniqueIndex>(hooked_function, std::forward<Args>(args)...);
			};

#define CallOriginalOpenGLActual(opengl_function_name, ...) \
			CallOriginalOpenGLActualUniqueWithName<fnv1a(STRINGIFY(opengl_function_name))>(CONSTEXPR_STRING(STRINGIFY(opengl_function_name)"OriginalActual"), opengl_function_name, __VA_ARGS__) \

#define CallOriginalOpenGL(opengl_function_name, ...) \
			CallOriginalOpenGLUniqueWithName<fnv1a(STRINGIFY(opengl_function_name))>(CONSTEXPR_STRING(STRINGIFY(opengl_function_name)"Original"), opengl_function_name, __VA_ARGS__) \

		}

		namespace
		{
			void CompatibilityInitialize()
			{
				// Buffers are set by default
				auto& context = Context::Get();
				auto& state = context.state;

				if (!context.compatibility_mode)
				{
					context.compatibility_mode = true;
					PanicIfError(context.buffer_manager.AllocateBuffer<VertexArrayObject>(0));
					PanicIfError(state.SetActiveVertexArrayObject(0));
				}
			}

			void CompabilityAllocateBuffer(GLuint buffer)
			{
				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				// Compatibility mode, buffer 0 is valid
				if (buffer == 0 && context.compatibility_mode)
				{
					if (auto buffer_or_err = context.buffer_manager.GetBuffer(buffer))
					{
						auto& buffer = *buffer_or_err;
					}
					else
					{
						llvm::consumeError(buffer_or_err.takeError());
						PanicIfError(context.buffer_manager.AllocateBuffer<VertexBufferObject>(buffer));
					}
				}
			}

			bool InvalidState()
			{
				auto& context = Context::Get();
				auto& state = context.state;

				if (!state.IsViewportValid())
				{
					return true;
				}
				return false;
			}
		}

		namespace OpenGLRedirection
		{
			void HOOK_CALL_CONVENTION GenVertexArrays(GLsizei n, GLuint* buffers)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(GenVertexArrays, n, buffers);

				auto& context = Context::Get();

				if (n < 0)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				if (buffers == nullptr)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}


				for (auto i = 0; i < n; i++)
				{
					auto& id = buffers[i];
					PanicIfError(context.buffer_manager.AllocateBuffer<VertexArrayObject>(id));
				}
			}

			void HOOK_CALL_CONVENTION GenBuffers(GLsizei n, GLuint* buffers)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(GenBuffers, n, buffers);

				auto& context = Context::Get();

				if (n < 0)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				if (buffers == nullptr)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				// assume buffer object for now unless bind buffer
				for (auto i = 0; i < n; i++)
				{
					auto& id = buffers[i];
					PanicIfError(context.buffer_manager.AllocateBuffer<VertexBufferObject>(id));
				}
			}

			void HOOK_CALL_CONVENTION DeleteBuffers(GLsizei n, const GLuint* buffers)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
			
				for (auto i = 0; i < n; i++)
				{
					auto id = buffers[i];
					if (id == 0)
					{
						continue;
					}
					state.AddMarkedDeleteBuffers(id);
				}

				/*
				if (RAYSTERIZER_OPENGL_INTERLOP_BUFFER_OPTIMIZATION)
				{
					for (auto i = 0; i < n; i++)
					{
						auto id = buffers[i];
						if (id == 0)
						{
							continue;
						}
						PanicIfError(Raysterizer::MiddleWare::OpenGLInteropManager::Get().DeallocateBuffer(id));
					}
				}
				else
				{
					CallOriginalOpenGL(DeleteBuffers, n, buffers);
				}

				auto& context = Context::Get();
				auto& state = context.state;

				// assume buffer object for now unless bind buffer
				for (auto i = 0; i < n; i++)
				{
					auto& id = buffers[i];
					if (id == 0)
					{
						continue;
					}
					if (RAYSTERIZER_OPENGL_INTERLOP_BUFFER_OPTIMIZATION)
					{
						// ignore...
						if (auto err = context.buffer_manager.DeleteBuffer(id))
						{
							llvm::consumeError(std::move(err));
						}
					}
					else
					{
						PanicIfError(context.buffer_manager.DeleteBuffer(id));
					}
				}
				*/
			}

			void HOOK_CALL_CONVENTION DeleteVertexArrays(GLsizei n, const GLuint* buffers)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DeleteVertexArrays, n, buffers);

				auto& context = Context::Get();

				if (n < 0)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				if (buffers == nullptr)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				// assume buffer object for now unless bind buffer
				for (auto i = 0; i < n; i++)
				{
					auto& id = buffers[i];
					if (id == 0)
					{
						continue;
					}
					PanicIfError(context.buffer_manager.DeleteVAO(id));
				}
			}

			void HOOK_CALL_CONVENTION BindBuffer(GLenum target, GLuint buffer)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BindBuffer, target, buffer);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				CompabilityAllocateBuffer(buffer);

				if (buffer > 0 || context.compatibility_mode)
				{
					if (target == GL_ARRAY_BUFFER)
					{
						PanicIfError(state.SetActiveVertexBufferObject(buffer));
					}
					else if (target == GL_ELEMENT_ARRAY_BUFFER)
					{
						PanicIfError(buffer_manager.ConvertVertexBufferObjectToElementBufferObject(buffer));
						PanicIfError(state.SetActiveElementBufferObject(buffer));
					}
					else if (target == GL_UNIFORM_BUFFER)
					{
						PanicIfError(buffer_manager.ConvertVertexBufferObjectToUniformBufferObject(buffer));
						PanicIfError(state.SetActiveUniformBufferObject(buffer));
					}
					else if (target == GL_PIXEL_PACK_BUFFER || target == GL_PIXEL_UNPACK_BUFFER)
					{
						// ignore...
					}
					else if (target == GL_SHADER_STORAGE_BUFFER)
					{
						PANIC("SHADER STORAGE BUFFER NOT SUPPORTED");
					}
					else if (target == GL_TEXTURE_BUFFER)
					{
						// ignore...
					}
					else
					{
						PANIC("Unknown buffer {}", target);
					}
				}
				else
				{
					// Reset bound buffer
					if (target == GL_ARRAY_BUFFER)
					{
						PanicIfError(state.SetActiveVertexBufferObject(buffer));
					}
					else if (target == GL_ELEMENT_ARRAY_BUFFER)
					{
						PanicIfError(state.SetActiveElementBufferObject(buffer));
					}
					else if (target == GL_UNIFORM_BUFFER)
					{
						PanicIfError(state.SetActiveUniformBufferObject(buffer));
					}
					else if (target == GL_PIXEL_PACK_BUFFER || target == GL_PIXEL_UNPACK_BUFFER)
					{
						// ignore...
					}
					else if (target == GL_TEXTURE_BUFFER)
					{
						// ignore...
					}
					else
					{
						PANIC("Unknown buffer {}", target);
					}
				}
			}

			void HOOK_CALL_CONVENTION BindVertexArray(GLuint array)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BindVertexArray, array);

				auto& context = Context::Get();
				auto& state = context.state;

				if (array > 0)
				{
					PanicIfError(state.SetActiveVertexArrayObject(array));

					// Set the ebo that's attached to the vao
					auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
					PanicIfError(state.SetActiveElementBufferObject(vao.GetBoundEBOID()));
				}
				else
				{
				}
			}

			void HOOK_CALL_CONVENTION BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				CallOriginalOpenGL(BufferData, target, size, data, usage);

				//if (InvalidState()) { return; }

				const phmap::flat_hash_set<GLenum> usages
				{
					{GL_STREAM_DRAW},
					{GL_STATIC_DRAW},
					{GL_DYNAMIC_DRAW},

					// ignore...
					{GL_STREAM_READ},
				};

				/*
				if (usage != GL_STREAM_DRAW && usage != GL_STATIC_DRAW && usage != GL_DYNAMIC_DRAW)
				{
					PANIC(GL_INVALID_ENUM);
					return;
				}
				*/

				if (size < 0)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				if (target == GL_ARRAY_BUFFER)
				{
					auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
					BlockHashedPointerView pointer_view(const_cast<void*>(data), 1, size);
					vbo.SetPointerView(pointer_view);
					vbo.SetUsage(usage);
				}
				else if (target == GL_ELEMENT_ARRAY_BUFFER)
				{
					auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());
					BlockHashedPointerView pointer_view(const_cast<void*>(data), 1, size);
					ebo.SetPointerView(pointer_view);
					ebo.SetUsage(usage);
				}
				else if (target == GL_UNIFORM_BUFFER)
				{
					auto& ubo = AssignOrPanic(state.GetActiveUniformBufferObject());
					BlockHashedPointerView pointer_view(const_cast<void*>(data), 1, size);
					ubo.SetPointerView(pointer_view);
					ubo.SetUsage(usage);
				}
				else if (target == GL_PIXEL_PACK_BUFFER || target == GL_PIXEL_UNPACK_BUFFER)
				{
					// ignore...
				}
				else
				{
					PANIC("Unknown target");
				}
			}

			phmap::flat_hash_set<GLuint> dirty_buffers;

			void HOOK_CALL_CONVENTION BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;
			
				CallOriginalOpenGL(BufferSubData, target, offset, size, data);

				//if (InvalidState()) { return; }

				if (size < 0)
				{
					PANIC(GL_INVALID_VALUE);
					return;
				}

				bool perform_hash = true;
				if (target == GL_ARRAY_BUFFER)
				{
					auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
					vbo.GetPointerView().CopyBytesFrom(data, size, offset, perform_hash);
				}
				else if (target == GL_ELEMENT_ARRAY_BUFFER)
				{
					auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());
					ebo.GetPointerView().CopyBytesFrom(data, size, offset, perform_hash);
				}
				else if (target == GL_UNIFORM_BUFFER)
				{
					auto& ubo = AssignOrPanic(state.GetActiveUniformBufferObject());
					ubo.GetPointerView().CopyBytesFrom(data, size, offset, perform_hash);
					
					if (0 && USE_NEW_SYNC_UBO_METHOD)
					{
						// Might be that the active program is not initialized -- just setting up initial buffers
						if (auto active_program_or_err = state.GetActiveProgram())
						{
							auto& active_program = *active_program_or_err;
							auto& pipeline_manager = context.GetPipelineManager();
							auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(active_program.GetId()));
							//auto buffer = AssignOrPanic(draw_calls.GetVariableInVulkanRef());

							// Sync if is uniform binding point
							auto& uniform_buffer_binding_points = buffer_manager.GetUniformBufferBindingPoints();
							for (auto i = 0; i < uniform_buffer_binding_points.size(); i++)
							{
								auto& uniform_buffer_binding_point = uniform_buffer_binding_points[i];
								auto ubbp_id = uniform_buffer_binding_point.GetBoundUBOID();
								if (ubo.GetId() == ubbp_id)
								{
									auto ubbp_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
									auto ubbp_size = uniform_buffer_binding_point.GetBoundUBOIDSize();
									const auto& uniform_block = AssignOrPanic(active_program.GetUniformBlock(i));
									auto buffer = AssignOrPanic(draw_calls.GetVariableInVulkanRef(uniform_block.name));

									PanicIfError(buffer->Copy(ubo.GetPointerView()));
								}
							}
						}
						else
						{
							ConsumeError(active_program_or_err.takeError());
						}
					}
				}
				else
				{
					PANIC("Unknown target");
				}
			}

			void UpdateVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				if (auto vbo_or_err = state.GetActiveVertexBufferObject())
				{
					auto& vbo = *vbo_or_err;
					if (auto vertex_attrib_pointer_or_err = vao.GetVertexAttribPointer(index))
					{
						auto& vertex_attrib_pointer = *vertex_attrib_pointer_or_err;
						vertex_attrib_pointer.index = index;
						vertex_attrib_pointer.size = size;
						vertex_attrib_pointer.type = type;
						vertex_attrib_pointer.normalized = normalized;
						vertex_attrib_pointer.total_size = stride;
						vertex_attrib_pointer.offset = (GLuint)pointer;
						vertex_attrib_pointer.associated_vbo = vbo.GetId();

						// vbo changes stride at this point
						auto& vbo_view = vbo.GetPointerView();
						if (vbo_view)
						{
							vbo_view.ChangeStride(stride);
						}

						vao.SetBoundVBOID(vbo.GetId());
						if (auto ebo_or_err = state.GetActiveElementBufferObject())
						{
							auto& ebo = *ebo_or_err;
							vao.SetBoundEBOID(ebo.GetId());
						}
						else
						{
							llvm::consumeError(ebo_or_err.takeError());
						}
					}
					else
					{
						PANIC("No vao index!");
						llvm::consumeError(vertex_attrib_pointer_or_err.takeError());
					}
				}
				else
				{
					//PANIC("No vbo!");
					llvm::consumeError(vbo_or_err.takeError());
				}
			}

			void HOOK_CALL_CONVENTION VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(VertexAttribPointer, index, size, type, normalized, stride, pointer);
				UpdateVertexAttribPointer(index, size, type, normalized, stride, pointer);
			}

			void HOOK_CALL_CONVENTION VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(VertexAttribIPointer, index, size, type, stride, pointer);
				UpdateVertexAttribPointer(index, size, type, false, stride, pointer);
			}

			void HOOK_CALL_CONVENTION VertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(VertexAttribLPointer, index, size, type, stride, pointer);
				UpdateVertexAttribPointer(index, size, type, false, stride, pointer);
			}

			void HOOK_CALL_CONVENTION EnableVertexAttribArray(GLuint index)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				if (auto vertex_attrib_pointer_or_err = vao.GetVertexAttribPointer(index))
				{
					auto& vertex_attrib_pointer = *vertex_attrib_pointer_or_err;
					vertex_attrib_pointer.enabled = true;
				}
				else
				{
					llvm::consumeError(vertex_attrib_pointer_or_err.takeError());
				}

				CallOriginalOpenGL(EnableVertexAttribArray, index);
			}

			void HOOK_CALL_CONVENTION DisableVertexAttribArray(GLuint index)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				if (auto vertex_attrib_pointer_or_err = vao.GetVertexAttribPointer(index))
				{
					auto& vertex_attrib_pointer = *vertex_attrib_pointer_or_err;
					vertex_attrib_pointer.enabled = false;
				}
				else
				{
					llvm::consumeError(vertex_attrib_pointer_or_err.takeError());
				}

				CallOriginalOpenGL(DisableVertexAttribArray, index);
			}

			void HOOK_CALL_CONVENTION VertexAttribDivisor(GLuint index, GLuint divisor)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				auto& vertex_attrib_pointer = AssignOrPanic(vao.GetVertexAttribPointer(index));
				vertex_attrib_pointer.divisor = divisor;

				CallOriginalOpenGL(VertexAttribDivisor, index, divisor);
			}

			GLuint HOOK_CALL_CONVENTION CreateShader(GLenum shaderType)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto shader_id = CallOriginalOpenGL(CreateShader, shaderType);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				if (shaderType == GL_VERTEX_SHADER)
				{
					PanicIfError(shader_manager.AllocateVertexShader(shader_id));
				}
				else if (shaderType == GL_FRAGMENT_SHADER)
				{
					PanicIfError(shader_manager.AllocateFragmentShader(shader_id));
				}
				else
				{
					PANIC("Shader type not supported: {}", shaderType);
				}

				return shader_id;
			}

			void HOOK_CALL_CONVENTION DeleteShader(GLuint shader)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DeleteShader, shader);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				PanicIfError(shader_manager.DeleteShader(shader));
			}

			void HOOK_CALL_CONVENTION ShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				std::stringstream shader_stream;

				for (auto i = 0; i < count; i++)
				{
					const auto* shader_source = string[i];
					if (length == nullptr)
					{
						shader_stream << shader_source;
					}
					else
					{
						auto source_length = length[i];
						if (source_length < 0)
						{
							shader_stream << shader_source;
						}
						else
						{
							auto shader_source_string = std::string(shader_source, source_length);
							shader_stream << shader_source_string;
						}
					}
				}

				auto shader_source = shader_stream.str();

				if (!context.opengl_version && !shader_source.empty())
				{
					//context.gl_version = "3.3.0";
					//context.opengl_version = 330;
					if (1)
					{
						context.gl_version = std::string((const char*)glGetString(GL_VERSION));
						{
							const auto& gl_version = context.gl_version;
							auto version_with_dots = RaysterizerEngine::Util::SplitString(gl_version, " ");
							auto versions = RaysterizerEngine::Util::SplitString(version_with_dots[0], ".");
							auto major = std::stoi(versions[0]);
							auto middle = std::stoi(versions[1]);
							auto minor = std::stoi(versions[2]);

							context.opengl_version = major * 100 + middle * 10 + minor * 1;
						}
						{
							using glGetStringi_type = const GLubyte* (*)(GLenum name, GLuint index);
							auto glGetStringi = (glGetStringi_type)AssignOrPanic(Raysterizer::Hooks::HookManager::Get().GetOriginalFunctionByName("wglSwapBuffers"));

							constexpr std::string_view gl_arb_compatibility = "GL_ARB_compatibility";

							GLint num_extensions{};
							glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
							for (int i = 0; i < num_extensions; i++)
							{
								const auto* extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
								if (extension)
								{
									if (gl_arb_compatibility == extension)
									{
										CompatibilityInitialize();
										break;
									}
								}
							}

							// Backup check
							if (!context.compatibility_mode)
							{
								// Will mention not in core as an error if enabled debug output
								DisableOpenGLDebugOutput();
								const auto* extensions_c_str = (const char*)glGetString(GL_EXTENSIONS);
								EnableOpenGLDebugOutput();

								if (extensions_c_str)
								{
									auto extension_str = std::string_view(extensions_c_str);
									auto extensions = RaysterizerEngine::Util::SplitString(extension_str, " ");

									for (const auto& extension : extensions)
									{
										if (gl_arb_compatibility == extension)
										{
											CompatibilityInitialize();
											break;
										}
									}
								}
							}
						}
					}

					auto shader_source_lines = RaysterizerEngine::Util::SplitString(shader_source, "\n");
					for (const auto& line : shader_source_lines)
					{
						auto first_sources = RaysterizerEngine::Util::SplitString(line, " ");
						if (first_sources.size() >= 2)
						{
							if (first_sources[0] == "#version")
							{
								const auto& version = first_sources[1];
								context.shader_version = std::stoi(version);
							}
						}
					}
				}

				auto WriteShader = [&](std::string extension)
				{
					const auto source_hash = std::to_string(XXH64(shader_source.data(), shader_source.length(), 0));
					std::ofstream f("opengl_shaders/" + source_hash + extension, std::ios::ate);
					if (f)
					{
						f << shader_source;
					}
				};

				if (auto vertex_shader_or_err = shader_manager.GetVertexShader(shader))
				{
					auto& vertex_shader = *vertex_shader_or_err;
					vertex_shader->SetSource(shader_source);

					WriteShader(".vert");
				}
				else
				{
					llvm::consumeError(vertex_shader_or_err.takeError());

					// probably fragment shader instead
					auto& fragment_shader = AssignOrPanic(shader_manager.GetFragmentShader(shader));
					fragment_shader->SetSource(shader_source);

					WriteShader(".frag");
				}

				CallOriginalOpenGL(ShaderSource, shader, count, string, length);
			}

			void HOOK_CALL_CONVENTION ShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length)
			{
				PANIC("Shader binary not supported!");
				CallOriginalOpenGL(ShaderBinary, count, shaders, binaryFormat, binary, length);
			}

			void HOOK_CALL_CONVENTION CompileShader(GLuint shader)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				//TODO: Do shader analysis at this point?
				CallOriginalOpenGL(CompileShader, shader);
			}

			void HOOK_CALL_CONVENTION GetShaderiv(GLuint shader, GLenum pname, GLint* params)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(GetShaderiv, shader, pname, params);
			}

			GLuint HOOK_CALL_CONVENTION CreateProgram()
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto program_id = CallOriginalOpenGL(CreateProgram);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				PanicIfError(shader_manager.AllocateProgram(program_id));

				return program_id;
			}

			void HOOK_CALL_CONVENTION DeleteProgram(GLuint program_id)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DeleteProgram, program_id);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				if (auto err = shader_manager.DeleteProgram(program_id))
				{
					LogError(err);
				}

				auto& pipeline_manager = context.GetPipelineManager();
				if (auto err = pipeline_manager.RemoveProgramToDrawCall(program_id))
				{
					LogError(err);
				}
			}

			void HOOK_CALL_CONVENTION AttachShader(GLuint program, GLuint shader)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(AttachShader, program, shader);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				auto& shader_program = AssignOrPanic(shader_manager.GetProgram(program));

				if (auto vertex_shader_or_err = shader_manager.GetVertexShader(shader))
				{
					auto& vertex_shader = *vertex_shader_or_err;
					shader_program.SetVertexShader(vertex_shader);
				}
				else
				{
					llvm::consumeError(vertex_shader_or_err.takeError());

					// probably fragment shader instead
					auto& fragment_shader = AssignOrPanic(shader_manager.GetFragmentShader(shader));
					shader_program.SetFragmentShader(fragment_shader);
				}
			}

			void PerformShaderReflection(GLuint program);

			void HOOK_CALL_CONVENTION LinkProgram(GLuint program)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				// TODO: Do shader analysis at this point?
				CallOriginalOpenGL(LinkProgram, program);

				PerformShaderReflection(program);
			}

			void HOOK_CALL_CONVENTION UseProgram(GLuint program)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(UseProgram, program);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;
				auto& buffer_manager = context.buffer_manager;
				auto& pipeline_manager = context.GetPipelineManager();

				PanicIfError(state.SetActiveProgram(program));

				// Sync any ubbp
				if (USE_NEW_SYNC_UBO_METHOD)
				{
					if (auto program_or_err = state.GetActiveProgram())
					{
						auto& program = AssignOrPanic(state.GetActiveProgram());
						for (auto& [index, uniform_block] : program.GetUniformBlockMapping())
						{
							auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(uniform_block.block_binding);
							auto uniform_buffer_binding_point_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
							auto uniform_buffer_binding_point_size = uniform_buffer_binding_point.GetBoundUBOIDSize();

							if (auto ubo_or_err = buffer_manager.GetUBO(uniform_buffer_binding_point.GetBoundUBOID()))
							{
								auto& ubo = *ubo_or_err;
								auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
								auto ubo_pointer_view = ubo.GetPointerView();

								// Sync if is uniform binding point
								auto ubbp_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
								auto ubbp_size = uniform_buffer_binding_point.GetBoundUBOIDSize();
								auto ubbp_pointer_view = PointerView(ubo_pointer_view.GetDataAs<uint8_t*>() + ubbp_offset, 1, ubbp_size);

								// TODO: this is inefficient copying buffer each time we switch programs
								// however, one needs to consider the current descriptor set as well.
								PanicIfError(draw_calls.CopyToBufferInVulkan(uniform_block.name, ubbp_offset, ubbp_pointer_view));
							}
							else
							{
								ConsumeError(ubo_or_err.takeError());
							}
						}
					}
					else
					{
						ConsumeError(program_or_err.takeError());
					}
				}
			}

			void HOOK_CALL_CONVENTION GetProgramiv(GLuint program, GLenum pname, GLint* params)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(GetProgramiv, program, pname, params);
			}

			void HOOK_CALL_CONVENTION GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(GetActiveUniform, program, index, bufSize, length, size, type, name);
			}

			GLint HOOK_CALL_CONVENTION GetUniformLocation(GLuint program, const GLchar* name)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto uniform_location_id = CallOriginalOpenGL(GetUniformLocation, program, name);
				return uniform_location_id;
			}

			void HOOK_CALL_CONVENTION GetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(GetActiveAttrib, program, index, bufSize, length, size, type, name);
			}

			GLint HOOK_CALL_CONVENTION GetAttribLocation(GLuint program, const GLchar* name)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto attrib_location_id = CallOriginalOpenGL(GetAttribLocation, program, name);
				return attrib_location_id;
			}

			namespace
			{
				GLuint HOOK_CALL_CONVENTION GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
				void HOOK_CALL_CONVENTION GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
				void HOOK_CALL_CONVENTION GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName);
			}

			void PerformShaderReflection(GLuint program)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				InitializeRaysterizer();

				// Perform reflection on program and set state
				auto& context = Context::Get();
				auto& state = context.state;
				auto& shader_manager = context.shader_manager;

				auto& shader_program = AssignOrPanic(shader_manager.GetProgram(program));

				// Perform shader analysis
				auto& vertex_shader = shader_program.GetVertexShader();
				const auto& vertex_shader_source = vertex_shader.GetSource();
				PanicIfError(vertex_shader.GetAnalyzer().Init(vertex_shader_source, EShLanguage::EShLangVertex));

				auto& fragment_shader = shader_program.GetFragmentShader();
				const auto& fragment_shader_source = fragment_shader.GetSource();
				PanicIfError(fragment_shader.GetAnalyzer().Init(fragment_shader_source, EShLanguage::EShLangFragment));

				PanicIfError(shader_program.LinkProgram());

				/////////////////////////////////////////

				const bool USE_INTERNAL_OPENGL = true;
				if (USE_INTERNAL_OPENGL)
				{
					{
						GLint max_length{};
						CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
						std::string name((max_length + 1), 0);

						GLint uniform_count{};
						CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_UNIFORMS, &uniform_count);

						for (GLuint i = 0; i < uniform_count; i++)
						{
							GLint size{};
							GLenum type{};

							//TODO: replace name with glsl parser
							auto* name_str = reinterpret_cast<GLchar*>(name.data());
							CallOriginalOpenGL(GetActiveUniform, program, i, max_length, nullptr, &size, &type, name_str);

							auto uniform_location = CallOriginalOpenGL(GetUniformLocation, program, name_str);
							if (uniform_location == -1)
							{
								continue;
							}

							/*
							// ignore values stored by fragment
							auto& vertex_uniforms = vertex_shader.GetAnalyzer().GetUniforms();
							auto& fragment_uniforms = fragment_shader.GetAnalyzer().GetUniforms();
							if (vertex_uniforms.contains(name_str) && !fragment_uniforms.contains(name_str))
							{
								PanicIfError(shader_program.AllocateUniform(uniform_location, name_str, type));
							}
							*/

							// CB0[0] -> CB0
							std::string name_view{ name_str };
							//fmt::print("UNIFORM {} {} {} {}\n", name_view, uniform_location, type, size);
							if (auto found = name_view.find("["); found != std::string::npos)
							{
								name_view = name_view.substr(0, found);

								// TODO: Allocate uniforms for CB0[1], CB0[2], ..., currently only CB0 
								/*
								for (auto j = 1; j < size; j++)
								{
									std::string name_index = fmt::format("{}[{}]", name_view, j);
									auto uniform_location = CallOriginalOpenGL(GetUniformLocation, program, name_index.c_str());
									if (uniform_location == -1)
									{
										continue;
									}
									PanicIfError(shader_program.AllocateUniform(uniform_location, name_index, type));
								}
								*/
							}

							PanicIfError(shader_program.AllocateUniform(uniform_location, name_view, type, size));
						}
					}

					{
						GLint uniform_count{};
						CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_UNIFORM_BLOCKS, &uniform_count);

						for (GLuint i = 0; i < uniform_count; i++)
						{
							GLint max_length{};
							CallOriginalOpenGL(GetActiveUniformBlockiv, program, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &max_length);
							std::string name((max_length + 1), 0);

							//TODO: replace name with glsl parser
							auto* name_str = reinterpret_cast<GLchar*>(name.data());
							CallOriginalOpenGL(GetActiveUniformBlockName, program, i, max_length, nullptr, name_str);

							std::string name_view{ name_str };
							PanicIfError(shader_program.AllocateUniformBlock(i, name_view));
						}
					}

					GLint max_length{};
					CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);
					std::string name((max_length + 1), 0);

					GLint attribute_count{};
					CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_ATTRIBUTES, &attribute_count);

					for (auto i = 0; i < attribute_count; i++)
					{
						GLint size{};
						GLenum type{};

						//TODO: replace name with glsl parser
						auto* name_str = reinterpret_cast<GLchar*>(name.data());
						CallOriginalOpenGL(GetActiveAttrib, program, i, max_length, nullptr, &size, &type, name_str);

						auto attrib_location = CallOriginalOpenGL(GetAttribLocation, program, name_str);
						if (attrib_location == -1)
						{
							continue;
						}

						PanicIfError(shader_program.AllocateAttrib(attrib_location, name_str, type));

						// Sync vertex shader attributes as well
						// This overwrites the default
						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						auto& pipeline_manager = context.GetPipelineManager();

						/*
						if (i != attrib_location)
						{
							PANIC("Attribute locations should be the same {} {}", i, attrib_location);
						}
						*/

						auto name_view = std::string_view(name_str);
					
						//name.erase(std::remove_if(std::begin(name), std::end(name), ::isspace), std::end(name));
						PanicIfError(vertex_analyzer.UpdatePipelineInputIndex(name_view, attrib_location));
						auto found_name = false;
						auto& vertex_pipeline_inputs = vertex_analyzer.GetPipelineInputs();
						for (auto& [input_name, pipeline_input] : vertex_pipeline_inputs)
						{
							if (input_name == name_view)
							{
								pipeline_input.SetReflectionAttribLocation(attrib_location);
								found_name = true;
								break;
							}
						}
						if (!found_name)
						{
							for (auto& [input_name, pipeline_input] : vertex_pipeline_inputs)
							{
								INFO("{}", input_name);
							}
							PANIC("UHNO");
						}
						/*
						if (auto found = pipeline_inputs.find(name_view); found != std::end(pipeline_inputs))
						{
							auto& [name, pipeline_input] = *found;
							pipeline_input.SetIndex(attrib_location);
						}
						else
						{
							PANIC("{} not found!\n", name_view);
						}
						*/
					}
				}
				else
				{
					glslang::TProgram& glsl_program = shader_program.GetProgram();
					glsl_program.dumpReflection();

					for (auto i = 0; i < glsl_program.getNumLiveUniformVariables(); i++)
					{
						const auto& uniform = glsl_program.getUniform(i);

						auto name_view = uniform.name;
						auto uniform_location = glsl_program.getReflectionIndex(name_view.data());
						auto type = uniform.glDefineType;
						auto size = uniform.size;

						if (uniform_location == -1)
						{
							continue;
						}

						//fmt::print("UNIFORM {} {} {} {}\n", name_view, uniform_location, type, size);
						if (auto found = name_view.find("["); found != std::string::npos)
						{
							name_view = name_view.substr(0, found);
						}

						//PanicIfError(shader_program.AllocateUniform(uniform_location, name_view, type, size));
					}

					for (auto i = 0; i < glsl_program.getNumLiveAttributes(); i++)
					{
						const auto& pipeline_input = glsl_program.getPipeInput(i);

						auto name_view = pipeline_input.name;
						auto attrib_location = glsl_program.getReflectionIndex(name_view.data());
						auto type = pipeline_input.glDefineType;
						auto size = pipeline_input.size;

						if (attrib_location == -1)
						{
							continue;
						}
						if (auto found = name_view.find("gl_"); found == 0)
						{
							continue;
						}
						//fmt::print("ATTRIB {} {} {} {}\n", name_view, attrib_location, type, size);

						if (auto found = name_view.find("["); found != std::string::npos)
						{
							name_view = name_view.substr(0, found);
						}

						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						//PanicIfError(vertex_analyzer.UpdatePipelineInputIndex(name_view, attrib_location));
					}

					for (auto i = 0; i < glsl_program.getNumLiveUniformBlocks(); i++)
					{
						const auto& uniform_block = glsl_program.getUniformBlock(i);
						//uniform_block.dump();

						const glslang::TType* attribute_type = uniform_block.getType();
						const auto* struct_type = attribute_type->getStruct();
						const auto& qualifier = attribute_type->getQualifier();
						const auto& structure = *attribute_type->getStruct();
						for (const auto& member : structure)
						{
							const auto* member_type = member.type;
							const auto& field_name = member_type->getFieldName();
						}
						//fmt::print("BLOCK {}\n", uniform_block.name);
					}

				}

				// TODO:
				// FILL IN THE NECESSARY DATA FOR UNIFORM BLOCKS IF THEY HAVE A BINDING
				// GetUniformBlockIndex/UniformBlockBinding are not needed if layout has binding

				auto& pipeline_manager = context.GetPipelineManager();
				PanicIfError(pipeline_manager.InsertProgramToDrawCall(program, vertex_shader, fragment_shader));
			}

			// UNIFORM BLOCKS
			namespace
			{
				GLuint HOOK_CALL_CONVENTION GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto result = CallOriginalOpenGL(GetUniformBlockIndex, program, uniformBlockName);

					if (result != UNDEFINED_ID)
					{
						auto index = result;

						auto& context = Context::Get();
						auto& shader_manager = context.shader_manager;
						auto& shader_program = AssignOrPanic(shader_manager.GetProgram(program));

						PanicIfError(shader_program.AllocateUniformBlock(index, uniformBlockName));
					}

					return result;
				}

				void HOOK_CALL_CONVENTION UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& shader_manager = context.shader_manager;
					auto& buffer_manager = context.buffer_manager;

					auto& shader_program = AssignOrPanic(shader_manager.GetProgram(program));

					auto& uniform_block = AssignOrPanic(shader_program.GetUniformBlock(uniformBlockIndex));
					uniform_block.block_binding = uniformBlockBinding;

					CallOriginalOpenGL(UniformBlockBinding, program, uniformBlockIndex, uniformBlockBinding);
				}

				void HOOK_CALL_CONVENTION GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
				{
					CallOriginalOpenGL(GetActiveUniformBlockiv, program, uniformBlockIndex, pname, params);
				}

				void HOOK_CALL_CONVENTION GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
				{
					CallOriginalOpenGL(GetActiveUniformBlockName, program, uniformBlockIndex, bufSize, length, uniformBlockName);
				}

				void HOOK_CALL_CONVENTION BindBufferBase(GLenum target, GLuint index, GLuint buffer)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& buffer_manager = context.buffer_manager;

					CompabilityAllocateBuffer(buffer);

					if (target == GL_UNIFORM_BUFFER)
					{
						PanicIfError(buffer_manager.ConvertVertexBufferObjectToUniformBufferObject(buffer));
						PanicIfError(state.SetActiveUniformBufferObject(buffer));

						auto& ubo = AssignOrPanic(buffer_manager.GetUBO(buffer));
						auto size = ubo.GetPointerView().GetTotalSize();

						auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(index);
						uniform_buffer_binding_point.SetBoundUBOID(buffer);
						uniform_buffer_binding_point.SetBoundUBOIDOffset(0);
						uniform_buffer_binding_point.SetBoundUBOIDSize(size);
					}
					else if (target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER)
					{
						PANIC("Not expecting binding");
					}
					else
					{
						PANIC("Target not supported: {}", target);
					}

					CallOriginalOpenGL(BindBufferBase, target, index, buffer);
				}

				void HOOK_CALL_CONVENTION BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& buffer_manager = context.buffer_manager;

					CompabilityAllocateBuffer(buffer);

					if (target == GL_UNIFORM_BUFFER)
					{
						PanicIfError(buffer_manager.ConvertVertexBufferObjectToUniformBufferObject(buffer));
						PanicIfError(state.SetActiveUniformBufferObject(buffer));

						auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(index);
						uniform_buffer_binding_point.SetBoundUBOID(buffer);
						uniform_buffer_binding_point.SetBoundUBOIDOffset(offset);
						uniform_buffer_binding_point.SetBoundUBOIDSize(size);
					}
					else if (target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER)
					{
						PANIC("Not expecting binding");
					}
					else
					{
						PANIC("Target not supported: {}", target);
					}

					CallOriginalOpenGL(BindBufferRange, target, index, buffer, offset, size);
				}

				void* HOOK_CALL_CONVENTION MapBuffer(GLenum target, GLenum access)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					void* result = CallOriginalOpenGL(MapBuffer, target, access);

					//if (InvalidState()) { return result; }

					if (target == GL_ARRAY_BUFFER)
					{
						auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
						vbo.SetMappedPointer(result, access);
					}
					else if (target == GL_ELEMENT_ARRAY_BUFFER)
					{
						auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());
						ebo.SetMappedPointer(result, access);
					}
					else if (target == GL_UNIFORM_BUFFER)
					{
						auto& ubo = AssignOrPanic(state.GetActiveUniformBufferObject());
						ubo.SetMappedPointer(result, access);
					}
					else if (target == GL_PIXEL_PACK_BUFFER || target == GL_PIXEL_UNPACK_BUFFER)
					{
						// ignore...
					}
					else
					{
						PANIC("Target not supported: {}", target);
					}

					return result;
				}

				void HOOK_CALL_CONVENTION UnmapBuffer(GLenum target)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;

					//if (InvalidState()) { return; }

					if (target == GL_ARRAY_BUFFER)
					{
						auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
						PanicIfError(vbo.SyncMappedPointerWithPointerView());
					}
					else if (target == GL_ELEMENT_ARRAY_BUFFER)
					{
						auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());
						PanicIfError(ebo.SyncMappedPointerWithPointerView());
					}
					else if (target == GL_UNIFORM_BUFFER)
					{
						auto& ubo = AssignOrPanic(state.GetActiveUniformBufferObject());
						PanicIfError(ubo.SyncMappedPointerWithPointerView());
					}
					else if (target == GL_PIXEL_PACK_BUFFER || target == GL_PIXEL_UNPACK_BUFFER)
					{
						// ignore...
					}
					else
					{
						PANIC("Target not supported: {}", target);
					}

					CallOriginalOpenGL(UnmapBuffer, target);
				}

				void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					void* result = CallOriginalOpenGL(MapBufferRange, target, offset, length, access);

					//if (InvalidState()) { return result; }

					if (target == GL_ARRAY_BUFFER)
					{
						// TODO: Need missing function to check vbo, why is this happening in roblox where I'm getting an ebo instead of vbo
						auto& pipeline_manager = context.GetPipelineManager();
						auto& buffer_manager = context.buffer_manager;
						if (pipeline_manager.GetGameType() == MiddleWare::GameType::Roblox)
						{
							auto gl_array_buffer_id = AssignOrPanic(state.GetOpenGLIdFromTarget(GL_ARRAY_BUFFER));
							if (auto buffer_or_err = buffer_manager.GetBuffer(gl_array_buffer_id))
							{
								auto& buffer = *buffer_or_err;
								// attempt to use ebo as vbo instead
								if (auto ebo = std::get_if<ElementBufferObject>(&buffer))
								{
									// convert to vbo
									buffer = VertexBufferObject(gl_array_buffer_id, *ebo);
								}
							}
							else
							{
								PANIC("ARRAY BUFFER not found");
							}
						}

						auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
						vbo.SetMappedPointer(result, access, offset, length);
					}
					else if (target == GL_ELEMENT_ARRAY_BUFFER)
					{
						auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());
						ebo.SetMappedPointer(result, access, offset, length);
					}
					else if (target == GL_UNIFORM_BUFFER)
					{
						auto& ubo = AssignOrPanic(state.GetActiveUniformBufferObject());
						ubo.SetMappedPointer(result, access, offset, length);
					}
					else if (target == GL_PIXEL_PACK_BUFFER || target == GL_PIXEL_UNPACK_BUFFER)
					{
						// ignore...
					}
					else if (target == GL_TEXTURE_BUFFER)
					{
						// ignore...
					}
					else
					{
						PANIC("Target not supported: {}", target);
					}

					return result;
				}
			}

			// UNIFORMS
			namespace
			{
				template<typename T>
				Expected<T&> GetUniformData(GLint location)
				{
					ScopedCPUProfileRaysterizerCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;

					auto& active_program = AssignOrPanic(state.GetActiveProgram());
					if (auto uniform_or_err = active_program.GetUniform(location))
					{
						auto& uniform = *uniform_or_err;

						if (auto* t = std::get_if<T>(&uniform.GetData()))
						{
							return *t;
						}
						else
						{
							return StringError("Uniform not same type {}", uniform.GetData().index());
						}
					}
					else
					{
						return uniform_or_err.takeError();
					}
					return StringError("Uniform data at {} location is not avaliable", location);
				}

				template<typename T>
				void SetUniformData(GLint location, T&& data)
				{
					ScopedCPUProfileRaysterizerCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;

					auto& active_program = AssignOrPanic(state.GetActiveProgram());
					if (auto uniform_or_err = active_program.GetUniform(location))
					{
						auto& uniform = *uniform_or_err;

						uniform.SetData(std::forward<T>(data));

						// Sync the variable in the spirv vm
						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(active_program.GetId()));
						auto& underlying_draw_calls = draw_calls;
						PanicIfError(underlying_draw_calls.SetVariable(uniform.name, std::forward<T>(data)));
					}
					else
					{
						llvm::consumeError(uniform_or_err.takeError());
					}
				}

				void SetUniformData(GLint location, SamplerId data)
				{
					ScopedCPUProfileRaysterizerCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;

					auto& active_program = AssignOrPanic(state.GetActiveProgram());
					if (auto uniform_or_err = active_program.GetUniform(location))
					{
						auto& uniform = *uniform_or_err;

						uniform.SetData(data);

						// Sync the variable in the spirv vm
						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(active_program.GetId()));
						auto& underlying_draw_calls = draw_calls;
						PanicIfError(underlying_draw_calls.SetVariable(uniform.name, data.sampler_id));
						//PanicIfError(underlying_draw_calls.SetVariableInVM(uniform.name, data.sampler_id));
						//PanicIfError(underlying_draw_calls.SetSampler(uniform.name, location, data.sampler_id));
					}
					else
					{
						llvm::consumeError(uniform_or_err.takeError());
					}
				}

				template<typename T>
				void SetUniformData(GLint location, std::vector<T>&& data)
				{
					ScopedCPUProfileRaysterizerCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;

					auto& active_program = AssignOrPanic(state.GetActiveProgram());
					if (auto uniform_or_err = active_program.GetUniform(location))
					{
						auto& uniform = *uniform_or_err;

						uniform.SetData(std::forward<T>(data));

						// Sync the variable in the spirv vm
						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(active_program.GetId()));
						auto& underlying_draw_calls = draw_calls.GetDrawCalls();
						PanicIfError(underlying_draw_calls.SetVariable(uniform.name, std::forward<T>(data)));
					}
					else
					{
						llvm::consumeError(uniform_or_err.takeError());
					}
				}

				void HOOK_CALL_CONVENTION Uniform1f(GLint location, GLfloat v0)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform1f, location, v0);

					SetUniformData(location, v0);
				}

				void HOOK_CALL_CONVENTION Uniform2f(GLint location, GLfloat v0, GLfloat v1)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform2f, location, v0, v1);

					SetUniformData(location, glm::vec2{ v0, v1 });
				}

				void HOOK_CALL_CONVENTION Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform3f, location, v0, v1, v2);

					SetUniformData(location, glm::vec3{ v0, v1, v2 });
				}

				void HOOK_CALL_CONVENTION Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform4f, location, v0, v1, v2, v3);

					SetUniformData(location, glm::vec4{ v0, v1, v2, v3 });
				}

				void HOOK_CALL_CONVENTION Uniform1i(GLint location, GLint v0)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform1i, location, v0);

					/*
					if (location = std::numeric_limits<decltype(location)>::max())
					{
						return;
					}
					*/

					auto& context = Context::Get();
					auto& state = context.state;

					auto& active_program = AssignOrPanic(state.GetActiveProgram());
					if (auto uniform_or_err = active_program.GetUniform(location))
					{
						auto& uniform = *uniform_or_err;
						auto existing_data = uniform.GetData();
						if (auto sampler = std::get_if<SamplerId>(&existing_data))
						{
							sampler->sampler_id = v0;
							auto& texture_manager = context.texture_manager;
							PanicIfError(active_program.BindActiveTextureUnitToName(v0, uniform.name));
							PanicIfError(active_program.SetLocationToTextureId(location, v0));
							SetUniformData(location, SamplerId{ GLuint(v0) });

							/*
							if (v0 != 0)
							{
								PanicIfError(texture_manager.BindActiveTextureUnitToName(v0, uniform.name));
							}
							else
							{
								PanicIfError(texture_manager.UnbindActiveTextureUnitToName(v0));
							}
							*/
						}
						else
						{
							SetUniformData(location, v0);
						}
					}
					else
					{
						// Consume error because uniform is not defined in shader
						ConsumeError(uniform_or_err.takeError());
					}
				}

				void HOOK_CALL_CONVENTION Uniform2i(GLint location, GLint v0, GLint v1)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform2i, location, v0, v1);

					SetUniformData(location, glm::ivec2{ v0, v1 });
				}

				void HOOK_CALL_CONVENTION Uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform3i, location, v0, v1, v2);

					SetUniformData(location, glm::ivec3{ v0, v1, v2 });
				}

				void HOOK_CALL_CONVENTION Uniform4i(GLint location, GLuint v0, GLint v1, GLint v2, GLint v3)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform4i, location, v0, v1, v2, v3);

					SetUniformData(location, glm::ivec4{ v0, v1, v2, v3 });
				}

				void HOOK_CALL_CONVENTION Uniform1ui(GLint location, GLuint v0)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform1ui, location, v0);

					SetUniformData(location, v0);
				}

				void HOOK_CALL_CONVENTION Uniform2ui(GLint location, GLuint v0, GLuint v1)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform2ui, location, v0, v1);

					SetUniformData(location, glm::uvec2{ v0, v1 });
				}

				void HOOK_CALL_CONVENTION Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform3ui, location, v0, v1, v2);

					SetUniformData(location, glm::uvec3{ v0, v1, v2 });
				}

				void HOOK_CALL_CONVENTION Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform4ui, location, v0, v1, v2, v3);

					SetUniformData(location, glm::uvec4{ v0, v1, v2, v3 });
				}

				void HOOK_CALL_CONVENTION Uniform1fv(GLint location, GLsizei count, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform1fv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, value[i]);
					}
				}

				void HOOK_CALL_CONVENTION Uniform2fv(GLint location, GLsizei count, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform2fv, location, count, value);

					if (count == 1)
					{
						Uniform2f(location, value[0], value[1]);
						return;
					}

					auto& data = AssignOrPanic(GetUniformData<std::vector<glm::vec2>>(location));

					constexpr auto division = 2;
					for (auto i = 0; i < count; i++)
					{
						auto val = value[i];
						data[(i / division)][(i % division)] = val;
					}

					SetUniformData(location, data);
				}

				void HOOK_CALL_CONVENTION Uniform3fv(GLint location, GLsizei count, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();
				
					CallOriginalOpenGL(Uniform3fv, location, count, value);

					if (count == 1)
					{
						Uniform3f(location, value[0], value[1], value[2]);
						return;
					}

					auto& data = AssignOrPanic(GetUniformData<std::vector<glm::vec3>>(location));

					constexpr auto division = 3;
					for (auto i = 0; i < count; i++)
					{
						auto val = value[i];
						data[(i / division)][(i % division)] = val;
					}

					SetUniformData(location, data);
				}

				void HOOK_CALL_CONVENTION Uniform4fv(GLint location, GLsizei count, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform4fv, location, count, value);

					if (count == 1)
					{
						Uniform4f(location, value[0], value[1], value[2], value[3]);
						return;
					}

					constexpr auto division = 4;
					for (auto i = 0; i < count / division; i++)
					{
						auto val = value[i];
						auto& data = AssignOrPanic(GetUniformData<glm::vec4>(location));
						data[(i % division)] = val;
						SetUniformData(location, data);
						location++;
					}
				}

				void HOOK_CALL_CONVENTION Uniform1iv(GLint location, GLsizei count, const GLint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform1iv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, value[i]);
					}
				}

				void HOOK_CALL_CONVENTION Uniform2iv(GLint location, GLsizei count, const GLint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform2iv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, glm::ivec2{ value[i * 2], value[i * 2 + 1] });
					}
				}

				void HOOK_CALL_CONVENTION Uniform3iv(GLint location, GLsizei count, const GLint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform3iv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, glm::ivec3{ value[i * 3], value[i * 3 + 1], value[i * 3 + 2] });
					}
				}

				void HOOK_CALL_CONVENTION Uniform4iv(GLint location, GLsizei count, const GLint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform4iv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, glm::ivec4{ value[i * 4], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3] });
					}
				}

				void HOOK_CALL_CONVENTION Uniform1uiv(GLint location, GLsizei count, const GLuint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform1uiv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, value[i]);
					}
				}

				void HOOK_CALL_CONVENTION Uniform2uiv(GLint location, GLsizei count, const GLuint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform2uiv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, glm::uvec2{ value[i * 2], value[i * 2 + 1] });
					}
				}

				void HOOK_CALL_CONVENTION Uniform3uiv(GLint location, GLsizei count, const GLuint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform3uiv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, glm::uvec3{ value[i * 3], value[i * 3 + 1], value[i * 3 + 2] });
					}
				}

				void HOOK_CALL_CONVENTION Uniform4uiv(GLint location, GLsizei count, const GLuint* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Uniform4uiv, location, count, value);

					for (auto i = 0; i < count; i++)
					{
						SetUniformData(location++, glm::uvec4{ value[i * 4], value[i * 4 + 1], value[i * 4 + 2], value[i * 4 + 3] });
					}
				}

				void HOOK_CALL_CONVENTION UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix2fv, location, count, transpose, value);

					for (auto i = 0; i < count; i++)
					{
						auto data = glm::mat2{
							value[i * 4 + 0],
							value[i * 4 + 1],
							value[i * 4 + 2],
							value[i * 4 + 3],
						};

						if (transpose == GL_TRUE)
						{
							data = glm::transpose(data);
						}

						SetUniformData(location++, data);
					}
				}

				void HOOK_CALL_CONVENTION UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix3fv, location, count, transpose, value);

					for (auto i = 0; i < count; i++)
					{
						auto data = glm::mat3{
							value[i * 9 + 0],
							value[i * 9 + 1],
							value[i * 9 + 2],
							value[i * 9 + 3],
							value[i * 9 + 4],
							value[i * 9 + 5],
							value[i * 9 + 6],
							value[i * 9 + 7],
							value[i * 9 + 8],
						};

						if (transpose == GL_TRUE)
						{
							data = glm::transpose(data);
						}

						SetUniformData(location++, data);
					}
				}

				void HOOK_CALL_CONVENTION UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix4fv, location, count, transpose, value);

					for (auto i = 0; i < count; i++)
					{
						auto data = glm::mat4{
							value[i * 16 + 0],
							value[i * 16 + 1],
							value[i * 16 + 2],
							value[i * 16 + 3],
							value[i * 16 + 4],
							value[i * 16 + 5],
							value[i * 16 + 6],
							value[i * 16 + 7],
							value[i * 16 + 8],
							value[i * 16 + 9],
							value[i * 16 + 10],
							value[i * 16 + 11],
							value[i * 16 + 12],
							value[i * 16 + 13],
							value[i * 16 + 14],
							value[i * 16 + 15],
						};

						if (transpose == GL_TRUE)
						{
							data = glm::transpose(data);
						}

						SetUniformData(location++, data);
					}
				}

				void HOOK_CALL_CONVENTION UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix2x3fv, location, count, transpose, value);
					PANIC("Unsupported");
				}

				void HOOK_CALL_CONVENTION UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix3x2fv, location, count, transpose, value);
					PANIC("Unsupported");
				}

				void HOOK_CALL_CONVENTION UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix2x4fv, location, count, transpose, value);
					PANIC("Unsupported");
				}

				void HOOK_CALL_CONVENTION UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix4x2fv, location, count, transpose, value);
					PANIC("Unsupported");
				}

				void HOOK_CALL_CONVENTION UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix3x4fv, location, count, transpose, value);
					PANIC("Unsupported");
				}

				void HOOK_CALL_CONVENTION UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(UniformMatrix4x3fv, location, count, transpose, value);
					PANIC("Unsupported");
				}
			}

			namespace
			{
				void HOOK_CALL_CONVENTION GenSamplers(GLsizei n, GLuint* samplers)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& sampler_manager = context.sampler_manager;

					CallOriginalOpenGL(GenSamplers, n, samplers);

					if (n < 0)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					if (samplers == nullptr)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					for (auto i = 0; i < n; i++)
					{
						auto& id = samplers[i];
						PanicIfError(sampler_manager.AllocateSampler(id));
					}
				}

				void HOOK_CALL_CONVENTION DeleteSamplers(GLsizei n, const GLuint* samplers)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& sampler_manager = context.sampler_manager;

					CallOriginalOpenGL(DeleteSamplers, n, samplers);

					if (n < 0)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					if (samplers == nullptr)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					for (auto i = 0; i < n; i++)
					{
						auto& id = samplers[i];
						PanicIfError(sampler_manager.DeallocateSampler(id));
					}
				}


				void HOOK_CALL_CONVENTION BindSampler(GLenum target, GLuint sampler)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(BindSampler, target, sampler);

					auto& context = Context::Get();
					auto& state = context.state;
					auto& sampler_manager = context.sampler_manager;
					auto& texture_manager = context.texture_manager;

					auto texture_unit = target;

					if (sampler != 0)
					{
						if (auto s_or_err = sampler_manager.GetSampler(sampler))
						{
							Sampler& s = *s_or_err;
							auto& active_program = AssignOrPanic(state.GetActiveProgram());
							PanicIfError(active_program.BindTextureUnitToSampler(texture_unit, sampler));
							PanicIfError(sampler_manager.BindSamplerToTextureUnit(sampler, texture_unit));
						}
						else
						{
							ConsumeError(s_or_err.takeError());
						}
					}

					// Check if texture unit is binded
					/*
					if (auto texture_unit_or_err = state.GetActiveTextureUnit())
					{
						auto texture_unit = *texture_unit_or_err;
						PanicIfError(texture_manager.BindTextureUnitToTextureId(texture_unit, texture));
					}
					else
					{
						llvm::consumeError(texture_unit_or_err.takeError());
					}

					PanicIfError(state.SetActiveTextureId(texture));
					if (texture != 0)
					{
						PanicIfError(texture_manager.SetTextureType(texture, target));
					}
					*/
					//Uniform1i(, texture);
				}

				void HOOK_CALL_CONVENTION SamplerParameteri(GLuint sampler, GLenum pname, GLint param)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(SamplerParameteri, sampler, pname, param);

					auto& context = Context::Get();
					auto& state = context.state;
					auto& sampler_manager = context.sampler_manager;
					auto& texture_manager = context.texture_manager;

					if (sampler != 0)
					{
						if (auto s_or_err = sampler_manager.GetSampler(sampler))
						{
							Sampler& s = *s_or_err;
							switch (pname)
							{
							case GL_TEXTURE_MIN_FILTER:
							{
								s.SetMinFilter(param);
								break;
							}
							case GL_TEXTURE_MAG_FILTER:
							{
								s.SetMagFilter(param);
								break;
							}
							case GL_TEXTURE_WRAP_T:
							{
								s.SetWrapT(param);
								break;
							}
							case GL_TEXTURE_WRAP_R:
							{
								s.SetWrapR(param);
								break;
							}
							case GL_TEXTURE_MIN_LOD:
							{
								s.SetMinLod(static_cast<float>(param));
								break;
							}
							case GL_TEXTURE_MAX_LOD:
							{
								s.SetMaxLod(static_cast<float>(param));
								break;
							}
							default:
							{
								break;
							}
							}
						}
						else
						{
							ConsumeError(s_or_err.takeError());
						}
					}

					// Check if texture unit is binded
					/*
					if (auto texture_unit_or_err = state.GetActiveTextureUnit())
					{
						auto texture_unit = *texture_unit_or_err;
						PanicIfError(texture_manager.BindTextureUnitToTextureId(texture_unit, texture));
					}
					else
					{
						llvm::consumeError(texture_unit_or_err.takeError());
					}

					PanicIfError(state.SetActiveTextureId(texture));
					if (texture != 0)
					{
						PanicIfError(texture_manager.SetTextureType(texture, target));
					}
					*/
					//Uniform1i(, texture);
				}

				void HOOK_CALL_CONVENTION GenTextures(GLsizei n, GLuint* textures)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& texture_manager = context.texture_manager;

					CallOriginalOpenGL(GenTextures, n, textures);

					if (n < 0)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					if (textures == nullptr)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					for (auto i = 0; i < n; i++)
					{
						auto& id = textures[i];
						PanicIfError(texture_manager.AllocateTexture(id));
					}
				}

				void HOOK_CALL_CONVENTION DeleteTextures(GLsizei n, const GLuint* textures)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& texture_manager = context.texture_manager;

					for (auto i = 0; i < n; i++)
					{
						auto id = textures[i];
						if (auto texture = texture_manager.GetTexture(id); !texture)
						{
							LogError(texture.takeError());
							return;
						}
					}

					CallOriginalOpenGL(DeleteTextures, n, textures);

					if (n < 0)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					if (textures == nullptr)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					for (auto i = 0; i < n; i++)
					{
						auto& id = textures[i];
						PanicIfError(texture_manager.DeallocateTexture(id));
					}
				}

				void HOOK_CALL_CONVENTION ActiveTexture(GLenum texture)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(ActiveTexture, texture);

					auto& context = Context::Get();
					auto& state = context.state;

					if (texture > GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS - 1)
					{
						PANIC("Texture out of bounds");
					}

					PanicIfError(state.SetActiveTextureUnit(texture));
				}

				void HOOK_CALL_CONVENTION BindTexture(GLenum target, GLuint texture)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(BindTexture, target, texture);

					auto& context = Context::Get();
					auto& state = context.state;
					auto& sampler_manager = context.sampler_manager;
					auto& texture_manager = context.texture_manager;

					// Check if texture unit is binded
					if (auto texture_unit_or_err = state.GetActiveTextureUnit())
					{
						auto texture_unit = *texture_unit_or_err;
						PanicIfError(texture_manager.BindTextureUnitToTextureId(texture_unit, texture));

						/*
						switch (context.GetPipelineManager().GetGameType())
						{
						case MiddleWare::GameType::Dolphin:
						{
							break;
						}
						default:
						{
							if (auto active_program_or_err = state.GetActiveProgram())
							{
								auto& active_program = *active_program_or_err;
								const auto& location_to_texture_id = active_program.GetLocationToTextureId();

								for (const auto& [location, texture_id] : location_to_texture_id)
								{
									if (auto sampler_id_or_err = active_program.GetSamplerFromTextureUnit(texture_id))
									{
										const auto& sampler_id = *sampler_id_or_err;
										if (auto sampler_or_err = sampler_manager.GetSampler(sampler_id))
										{
											auto& sampler = *sampler_or_err;
											sampler.BindTextureUnitToTextureId(texture_unit, texture);
										}
										else
										{
											ConsumeError(sampler_or_err.takeError());
										}

										break;
									}
									else
									{
										ConsumeError(sampler_id_or_err.takeError());
									}
								}
							}
							else
							{
								ConsumeError(active_program_or_err.takeError());
							}
							break;
						}
						}
						*/
					}
					else
					{
						ConsumeError(texture_unit_or_err.takeError());
					}

					if (target == GL_TEXTURE_BUFFER)
					{
						return;
					}

					PanicIfError(state.SetActiveTextureId(texture));
					if (texture != 0)
					{
						PanicIfError(texture_manager.SetTextureType(texture, target));
					}
				}

				template<typename T>
				void SetTextureInformation(GLuint texture_id, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* data)
				{
					auto& context = Context::Get();
					auto& texture_manager = context.texture_manager;

					auto& texture = AssignOrPanic(texture_manager.GetTextureAs<T>(texture_id));
					texture.SetLevelOfDetail(level);
					texture.SetWidth(width);
					texture.SetHeight(height);
					texture.SetDepth(depth);
					texture.SetColorFormat(internalformat);
					texture.SetPixelFormat(format);
					texture.SetPixelDataType(type);

					if (RAYSTERIZER_OPENGL_INTERLOP_TEXTURE_OPTIMIZATION)
					{
					}
					else
					{
						PanicIfError(texture.InitializeBuffer());
						PanicIfError(texture.SetData(data, width, height, depth));
					}
				}

				void HOOK_CALL_CONVENTION TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& texture_manager = context.texture_manager;
					auto texture_id = AssignOrPanic(state.GetActiveTextureId());

					CallOriginalOpenGL(TexStorage2D, target, levels, internalformat, width, height);

					switch (target)
					{
					case GL_TEXTURE_2D:
					{
						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<Texture2D>(texture_id));
						texture.SetLevelOfDetail(levels);
						texture.SetWidth(width);
						texture.SetHeight(height);
						texture.SetColorFormat(internalformat);
						PanicIfError(texture.InitializeBuffer());
						break;
					}
					case GL_TEXTURE_CUBE_MAP:
					{
						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<TextureCubeMap>(texture_id));
						texture.SetLevelOfDetail(levels);
						texture.SetWidth(width);
						texture.SetHeight(height);
						texture.SetColorFormat(internalformat);
						PanicIfError(texture.InitializeBuffer());
						break;
					}
					default:
					{
						PANIC("Other targets not supported");
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& texture_manager = context.texture_manager;
					auto texture_id = AssignOrPanic(state.GetActiveTextureId());

					CallOriginalOpenGL(TexStorage3D, target, levels, internalformat, width, height, depth);

					switch (target)
					{
					case GL_TEXTURE_2D_ARRAY:
					{
						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<Texture2DArray>(texture_id));
						texture.SetLevelOfDetail(levels);
						texture.SetWidth(width);
						texture.SetHeight(height);
						texture.SetDepth(depth);
						texture.SetColorFormat(internalformat);
						PanicIfError(texture.InitializeBuffer());
						break;
					}
					case GL_TEXTURE_3D:
					{
						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<Texture3D>(texture_id));
						texture.SetLevelOfDetail(levels);
						texture.SetWidth(width);
						texture.SetHeight(height);
						texture.SetDepth(depth);
						texture.SetColorFormat(internalformat);
						PanicIfError(texture.InitializeBuffer());
						break;
					}
					default:
					{
						PANIC("Other targets not supported");
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION TexSubImage2D(GLenum target,
					GLint level,
					GLint xoffset,
					GLint yoffset,
					GLsizei width,
					GLsizei height,
					GLenum format,
					GLenum type,
					const void* pixels)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& texture_manager = context.texture_manager;
					auto texture_id = AssignOrPanic(state.GetActiveTextureId());

					CallOriginalOpenGL(TexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, pixels);

					switch (target)
					{
					case GL_TEXTURE_2D:
					{
						/*
						if (xoffset != 0 || yoffset != 0)
						{
							PANIC("Sub texturing not supported...");
						}
						*/

						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<Texture2D>(texture_id));
						texture.SetLevelOfDetail(level);
						texture.SetPixelFormat(format);
						texture.SetPixelDataType(type);

						PanicIfError(texture.InitializeBuffer());
						PanicIfError(texture.SetData(pixels, width, height, 1, xoffset, yoffset));
						break;
					}
					case GL_TEXTURE_CUBE_MAP:
					case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
					case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
					case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
					{
						// TODO:
						// Implement
						break;
					}
					default:
					{
						PANIC("Other targets not supported");
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION TexSubImage3D(GLenum target,
					GLint level,
					GLint xoffset,
					GLint yoffset,
					GLint zoffset,
					GLsizei width,
					GLsizei height,
					GLsizei depth,
					GLenum format,
					GLenum type,
					const void* pixels)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& texture_manager = context.texture_manager;
					auto texture_id = AssignOrPanic(state.GetActiveTextureId());

					CallOriginalOpenGL(TexSubImage3D, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);

					switch (target)
					{
					case GL_TEXTURE_2D_ARRAY:
					{
						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<Texture2DArray>(texture_id));
						texture.SetLevelOfDetail(level);
						texture.SetPixelFormat(format);
						texture.SetPixelDataType(type);

						PanicIfError(texture.InitializeBuffer());
						PanicIfError(texture.SetData(pixels, width, height, depth, xoffset, yoffset, zoffset));
						break;
					}
					case GL_TEXTURE_3D:
					{
						auto& texture = AssignOrPanic(texture_manager.GetTextureAs<Texture3D>(texture_id));
						texture.SetLevelOfDetail(level);
						texture.SetPixelFormat(format);
						texture.SetPixelDataType(type);

						PanicIfError(texture.InitializeBuffer());
						PanicIfError(texture.SetData(pixels, width, height, depth, xoffset, yoffset, zoffset));
						break;
					}
					default:
					{
						PANIC("Other targets not supported");
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& texture_manager = context.texture_manager;
					auto texture_id = AssignOrPanic(state.GetActiveTextureId());

					CallOriginalOpenGL(TexImage2D, target, level, internalformat, width, height, border, format, type, data);

					switch (target)
					{
					case GL_TEXTURE_2D:
					{
						SetTextureInformation<Texture2D>(texture_id, level, internalformat, width, height, 1, border, format, type, data);
						break;
					}
					// Ignore...
					case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
					case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
					case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
					case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
					{
						break;
					}
					default:
					{
						PANIC("Other targets not supported");
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* data)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto& context = Context::Get();
					auto& state = context.state;
					auto& texture_manager = context.texture_manager;
					auto texture_id = AssignOrPanic(state.GetActiveTextureId());

					CallOriginalOpenGL(TexImage3D, target, level, internalformat, width, height, depth, border, format, type, data);

					switch (target)
					{
					case GL_TEXTURE_2D_ARRAY:
					{
						SetTextureInformation<Texture2DArray>(texture_id, level, internalformat, width, height, depth, border, format, type, data);
						break;
					}
					//Ignore...
					case GL_TEXTURE_3D:
					{
						break;
					}
					default:
					{
						PANIC("Other targets not supported");
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION GenerateMipmap(GLenum target)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(GenerateMipmap, target);
				}

				void HOOK_CALL_CONVENTION TexParameteri(GLenum target, GLenum pname, GLint param)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(TexParameteri, target, pname, param);
				}

				void HOOK_CALL_CONVENTION GenFramebuffers(GLsizei n, GLuint* ids)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					InitializeRaysterizer();

					CallOriginalOpenGL(GenFramebuffers, n, ids);

					auto& context = Context::Get();
					auto& frame_buffer_manager = context.frame_buffer_manager;
					auto& pipeline_manager = context.GetPipelineManager();

					if (n < 0)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					if (ids == nullptr)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					for (auto i = 0; i < n; i++)
					{
						auto& id = ids[i];
						PanicIfError(frame_buffer_manager.AllocateFrameBufferObject(id));
						PanicIfError(pipeline_manager.CreateGBufferPass(id));
					}
				}

				void HOOK_CALL_CONVENTION DeleteFramebuffers(GLsizei n, const GLuint* ids)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(DeleteFramebuffers, n, ids);

					auto& context = Context::Get();
					auto& frame_buffer_manager = context.frame_buffer_manager;
					auto& pipeline_manager = context.GetPipelineManager();

					if (n < 0)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					if (ids == nullptr)
					{
						PANIC(GL_INVALID_VALUE);
						return;
					}

					for (auto i = 0; i < n; i++)
					{
						auto& id = ids[i];
						PanicIfError(frame_buffer_manager.DeallocateFrameBufferObject(id));
						if (id != 0)
						{
							PanicIfError(pipeline_manager.DeleteGBufferPass(id));
						}
					}
				}

				void HOOK_CALL_CONVENTION FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(FramebufferTextureLayer, target, attachment, texture, level, layer);

					auto& context = Context::Get();
					auto& state = context.state;
					auto& frame_buffer_manager = context.frame_buffer_manager;

					if (level != 0)
					{
						PANIC("Expect level to be 0, instead it is {}", level);
					}

					if (layer != 0)
					{
						PANIC("Expect layer to be 0, instead it is {}", layer);
					}

					switch (target)
					{
					case GL_FRAMEBUFFER:
					{
						auto fbo_id = AssignOrPanic(state.GetActiveFrameBufferObjectId());
						auto fbo = AssignOrPanic(frame_buffer_manager.GetFrameBufferObject(fbo_id));

						switch (attachment)
						{
						case GL_COLOR_ATTACHMENT0:
						{
							fbo.color_attachment_texture = texture;
							break;
						}
						case GL_DEPTH_ATTACHMENT:
						{
							fbo.depth_attachment_texture = texture;
							break;
						}
						default:
						{
							PANIC("{:08X} attachment not supported", attachment);
							break;
						}
						}
						break;
					}
					default:
					{
						PANIC("{:08X} target not supported", target);
						break;
					}
					}
				}

				void HOOK_CALL_CONVENTION FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					//CallOriginalOpenGL(FramebufferTexture, target, attachment, texture, level);
					FramebufferTextureLayer(target, attachment, texture, level, 0);
				}

				void HOOK_CALL_CONVENTION BindFramebuffer(GLenum target, GLuint framebuffer)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(BindFramebuffer, target, framebuffer);

					auto& context = Context::Get();
					auto& state = context.state;
					auto& frame_buffer_manager = context.frame_buffer_manager;
					auto& pipeline_manager = context.GetPipelineManager();

					switch (target)
					{
					case GL_DRAW_FRAMEBUFFER:
					case GL_FRAMEBUFFER:
					{
						auto previous_fbo_id = AssignOrPanic(state.GetActiveFrameBufferObjectId());
						if (previous_fbo_id != framebuffer)
						{
							auto previous_g_buffer_pass = AssignOrPanic(pipeline_manager.GetGBufferPass(previous_fbo_id));
							previous_g_buffer_pass->EndRender(pipeline_manager.frame_command_buffer);

							PanicIfError(state.SetActiveFrameBufferObjectId(framebuffer));
							PanicIfError(pipeline_manager.SetActiveGBufferPass(framebuffer));
							auto g_buffer_pass = AssignOrPanic(pipeline_manager.GetGBufferPass(framebuffer));
							g_buffer_pass->BeginRender(pipeline_manager.frame_command_buffer);
						}
						break;
					}
					// ignore
					case GL_READ_FRAMEBUFFER:
					{
						break;
					}
					default:
					{
						PANIC("{:08X} target not supported", target);
						break;
					}
					}
				}
			}

			namespace
			{
				void SyncUniformBufferObjects()
				{
					if (USE_NEW_SYNC_UBO_METHOD)
					{
						ScopedCPUProfileRaysterizerCurrentFunction();

						// Perform reflection on program and set state
						auto& context = Context::Get();
						auto& state = context.state;
						auto& shader_manager = context.shader_manager;
						auto& buffer_manager = context.buffer_manager;

						auto& program = AssignOrPanic(state.GetActiveProgram());
						auto& shader_program = AssignOrPanic(shader_manager.GetProgram(program.GetId()));

						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
						auto& underlying_draw_calls = draw_calls;

						// Perform shader analysis
						auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
						auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());

						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						auto& fragment_analyzer = fragment_shader.GetAnalyzer();

						auto& analyzer = vertex_analyzer;
						const auto& uniform_blocks = analyzer.GetUniformBlocks();

						if (pipeline_manager.GetGameType() == MiddleWare::GameType::Dolphin || pipeline_manager.GetGameType() == MiddleWare::GameType::Roblox)
						{
							for (auto& [index, uniform_block] : shader_program.GetUniformBlockMapping())
							{
								auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(uniform_block.block_binding);
								auto uniform_buffer_binding_point_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
								auto uniform_buffer_binding_point_size = uniform_buffer_binding_point.GetBoundUBOIDSize();

								auto& ubo = AssignOrPanic(buffer_manager.GetUBO(uniform_buffer_binding_point.GetBoundUBOID()));

								auto& ubo_ptr_view = ubo.GetPointerView();
								auto ubo_total_size = ubo_ptr_view.GetTotalSize();
								auto ubo_ptr = ubo_ptr_view.GetDataAs<uintptr_t>();

								auto ubo_ptr_offseted = ubo_ptr + uniform_buffer_binding_point_offset;

								auto copy_pointer = PointerView((void*)ubo_ptr_offseted, (GLuint)uniform_buffer_binding_point_size, 1);
								PanicIfError(underlying_draw_calls.CopyToBufferInVulkan(uniform_block.name, 0, copy_pointer));
							}
						}

						for (auto& [name, uniform] : analyzer.GetUniforms())
						{
							auto& object_reflection = uniform.GetObjectReflection();
							const auto& glsl_type = *uniform.GetGLSLType();
							auto offset = object_reflection.offset;
							auto size = Raysterizer::OpenGL::Util::GLenumToSize(uniform.GetGLType());
							auto array_size = 0;
							std::vector<uint32_t> array_sizes;
							if (glsl_type.containsArray())
							{
								const auto& glsl_array_sizes = *glsl_type.getArraySizes();
								auto glsl_array_size = glsl_array_sizes.getCumulativeSize();
								array_size = glsl_array_size;

								for (auto i = 0; i < glsl_array_sizes.getNumDims(); i++)
								{
									auto array_size = glsl_array_sizes.getDimSize(i);
									array_sizes.emplace_back(array_size);
								}
							}

							// currently this makes a copy per every uniform
							// what is suppose to happen is that the uniform buffers should be shared
							if (uniform.IsPartOfUniformBlock())
							{
								auto& uniform_block_name = uniform.GetParentUniformBlockName();
								if (auto found = uniform_blocks.find(uniform_block_name); found != std::end(uniform_blocks))
								{
									const auto& [_, vertex_uniform_block] = *found;
									for (auto& [index, uniform_block] : shader_program.GetUniformBlockMapping())
									{
										if (uniform_block_name == uniform_block.name)
										{
											auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(uniform_block.block_binding);
											auto uniform_buffer_binding_point_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
											auto uniform_buffer_binding_point_size = uniform_buffer_binding_point.GetBoundUBOIDSize();

											auto& ubo = AssignOrPanic(buffer_manager.GetUBO(uniform_buffer_binding_point.GetBoundUBOID()));

											auto& ubo_ptr_view = ubo.GetPointerView();
											auto ubo_total_size = ubo_ptr_view.GetTotalSize();
											auto ubo_ptr = ubo_ptr_view.GetDataAs<uintptr_t>();
											ubo_ptr += offset;

											auto ubo_ptr_offseted = ubo_ptr + uniform_buffer_binding_point_offset;
											
											/*
											if (pipeline_manager.GetGameType() == MiddleWare::GameType::Dolphin)
											{
												auto copy_pointer = PointerView((void*)(ubo_ptr_view.GetDataAs<uintptr_t>() + uniform_buffer_binding_point_offset), (GLuint)ubo_total_size, 1);
												PanicIfError(underlying_draw_calls.CopyToBufferInVulkan(uniform_block.name, 0, copy_pointer));
											}
											*/

											auto SetVariableInVulkanAndVM = [&](const auto& data)
											{
												if (auto err = underlying_draw_calls.SetVariableInVM(name, data))
												{
													ConsumeError(err);
													auto struct_name = fmt::format("{}.{}", uniform_block_name, name);
													if (auto err2 = underlying_draw_calls.SetVariableInVM(struct_name, data))
													{
														ConsumeError(err2);
													}
												}
											};

											// Different names in VM and Vulkan reflection
											// VM -> uniform name -- projection
											// Vulkan shader -> Matrices.projection
											if (array_size != 0)
											{
												auto* current_ptr = reinterpret_cast<glm::vec4*>(ubo_ptr_offseted);
												if (array_sizes.size() != 1)
												{
													PANIC("Assume 1");
												}

												// This only works for single arrays
												// need to fix for multi dimensional arrays

												// Sync ubo with spirv vm
												if (size == sizeof(glm::vec4))
												{
													for (const auto& array_size : array_sizes)
													{
														for (auto i = 0; i < array_size; i++)
														{
															auto member_name = object_reflection.name;
															auto left_bracket = member_name.find("[");
															auto right_bracket = member_name.find("]");
															if (left_bracket != std::string::npos && right_bracket != std::string::npos)
															{
																auto left = std::begin(member_name) + left_bracket + 1;
																auto right = std::begin(member_name) + right_bracket;
																if (right < left)
																{
																	PANIC("Left should be smaller than right for array access");
																}
																member_name.replace(left, right, fmt::format("{}", i));
															}
															else
															{
																member_name = fmt::format("{}[{}]", name, i);
															}
															auto data = current_ptr[i];

															PanicIfError(underlying_draw_calls.SetVariableInVM(member_name, data));
														}
													}
												}
											}
											else if (size == sizeof(bool))
											{
												auto data = *reinterpret_cast<bool*>(ubo_ptr_offseted);
												SetVariableInVulkanAndVM(data);
											}
											else if (size == sizeof(float))
											{
												auto data = *reinterpret_cast<float*>(ubo_ptr_offseted);
												SetVariableInVulkanAndVM(data);
											}
											else if (size == sizeof(glm::vec2))
											{
												auto data = *reinterpret_cast<glm::vec2*>(ubo_ptr_offseted);
												SetVariableInVulkanAndVM(data);
											}
											else if (size == sizeof(glm::vec3))
											{
												auto data = *reinterpret_cast<glm::vec3*>(ubo_ptr_offseted);
												SetVariableInVulkanAndVM(data);
											}
											else if (size == sizeof(glm::vec4))
											{
												auto data = *reinterpret_cast<glm::vec4*>(ubo_ptr_offseted);
												SetVariableInVulkanAndVM(data);
											}
											else if (size == sizeof(glm::mat4))
											{
												auto data = *reinterpret_cast<glm::mat4*>(ubo_ptr_offseted);
												SetVariableInVulkanAndVM(data);
											}
											else
											{
												PANIC("No matching datasize");
											}
											break;
										}
									}
								}
							}
						}
					}

					if(!USE_NEW_SYNC_UBO_METHOD)
					{
						ScopedCPUProfileRaysterizerCurrentFunction();

						// Perform reflection on program and set state
						auto& context = Context::Get();
						auto& state = context.state;
						auto& shader_manager = context.shader_manager;
						auto& buffer_manager = context.buffer_manager;

						auto& program = AssignOrPanic(state.GetActiveProgram());
						auto& shader_program = AssignOrPanic(shader_manager.GetProgram(program.GetId()));

						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
						auto& underlying_draw_calls = draw_calls;

						// Perform shader analysis
						auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
						auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());

						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						auto& fragment_analyzer = fragment_shader.GetAnalyzer();

						auto SyncUniforms = [&](Raysterizer::Analysis::GLSLAnalyzer& analyzer, bool set_in_vm = true)
						{
							const auto& uniform_blocks = analyzer.GetUniformBlocks();
							for (auto& [name, uniform] : analyzer.GetUniforms())
							{
								auto& object_reflection = uniform.GetObjectReflection();
								const auto& glsl_type = *uniform.GetGLSLType();
								auto offset = object_reflection.offset;
								auto size = Raysterizer::OpenGL::Util::GLenumToSize(uniform.GetGLType());
								auto array_size = 0;
								std::vector<uint32_t> array_sizes;
								if (glsl_type.containsArray())
								{
									const auto& glsl_array_sizes = *glsl_type.getArraySizes();
									auto glsl_array_size = glsl_array_sizes.getCumulativeSize();
									array_size = glsl_array_size;

									for (auto i = 0; i < glsl_array_sizes.getNumDims(); i++)
									{
										auto array_size = glsl_array_sizes.getDimSize(i);
										array_sizes.emplace_back(array_size);
									}
								}

								// currently this makes a copy per every uniform
								// what is suppose to happen is that the uniform buffers should be shared
								if (uniform.IsPartOfUniformBlock())
								{
									auto& uniform_block_name = uniform.GetParentUniformBlockName();
									if (auto found = uniform_blocks.find(uniform_block_name); found != std::end(uniform_blocks))
									{
										const auto& [_, vertex_uniform_block] = *found;
										for (auto& [index, uniform_block] : shader_program.GetUniformBlockMapping())
										{
											if (uniform_block_name == uniform_block.name)
											{
												auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(uniform_block.block_binding);
												auto uniform_buffer_binding_point_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
												auto uniform_buffer_binding_point_size = uniform_buffer_binding_point.GetBoundUBOIDSize();

												auto& ubo = AssignOrPanic(buffer_manager.GetUBO(uniform_buffer_binding_point.GetBoundUBOID()));

												auto& ubo_ptr_view = ubo.GetPointerView();
												auto ubo_total_size = ubo_ptr_view.GetTotalSize();
												auto ubo_ptr = ubo_ptr_view.GetDataAs<uintptr_t>();
												ubo_ptr += offset;

												auto ubo_ptr_offseted = ubo_ptr + uniform_buffer_binding_point_offset;

												auto SetVariableInVulkanAndVM = [&](const auto& data)
												{
													if (auto err = underlying_draw_calls.SetVariableInVulkan(name, data))
													{
														ConsumeError(err);
														auto struct_name = fmt::format("{}.{}", uniform_block_name, name);
														PanicIfError(underlying_draw_calls.SetVariableInVulkan(struct_name, data));
													}
													if (set_in_vm)
													{
														if (auto err = underlying_draw_calls.SetVariableInVM(name, data))
														{
															ConsumeError(err);
															auto struct_name = fmt::format("{}.{}", uniform_block_name, name);
															if (auto err2 = underlying_draw_calls.SetVariableInVM(struct_name, data))
															{
																ConsumeError(err2);
															}
														}
													}
												};

												// Different names in VM and Vulkan reflection
												// VM -> uniform name -- projection
												// Vulkan shader -> Matrices.projection
												if (array_size != 0)
												{
													auto* current_ptr = reinterpret_cast<glm::vec4*>(ubo_ptr_offseted);
													if (array_sizes.size() != 1)
													{
														PANIC("Assume 1");
													}

													// This only works for single arrays
													// need to fix for multi dimensional arrays

													// Sync ubo with spirv vm
													if (set_in_vm && size == sizeof(glm::vec4))
													{
														for (const auto& array_size : array_sizes)
														{
															for (auto i = 0; i < array_size; i++)
															{
																auto member_name = object_reflection.name;
																auto left_bracket = member_name.find("[");
																auto right_bracket = member_name.find("]");
																if (left_bracket != std::string::npos && right_bracket != std::string::npos)
																{
																	auto left = std::begin(member_name) + left_bracket + 1;
																	auto right = std::begin(member_name) + right_bracket;
																	if (right < left)
																	{
																		PANIC("Left should be smaller than right for array access");
																	}
																	member_name.replace(left, right, fmt::format("{}", i));
																}
																else
																{
																	member_name = fmt::format("{}[{}]", name, i);
																}
																auto data = current_ptr[i];

																PanicIfError(underlying_draw_calls.SetVariableInVM(member_name, data));
															}
														}
													}

													// Sync ubo with vulkan
													CMShared<RaysterizerEngine::Buffer>& vulkan_buffer = AssignOrPanic(underlying_draw_calls.GetVariableInVulkanRef(name));
													if (vulkan_buffer->GetSize() != uniform_buffer_binding_point_size)
													{
														const auto& buffer_create_info = vulkan_buffer->buffer_create_info;
														vulkan_buffer = AssignOrPanic(c.GetRenderFrame().CreateBuffer(MemoryUsage::CpuToGpu, buffer_create_info.buffer_usage_flags, uniform_buffer_binding_point_size, true));
													}

													auto* buffer = vulkan_buffer->Map();
													memcpy(buffer, (const void*)ubo_ptr_offseted, uniform_buffer_binding_point_size);

													//vulkan_buffer->Unmap();

													/*
													// Could be mapped at an offset of big buffer containing multiple variables inside it
													// ex, [.......................] = buffer
													//     [CB0  ]   [CB1 ] [CB2]
													if (vulkan_buffer->Size() != ubo_total_size)
													{
														//vulkan_buffer->Init(ubo_total_size, vulkan_buffer->Usage(), vulkan_buffer->MemoryType());
														*vulkan_buffer = Raysterizer::Vulkan::Buffer(ubo_total_size,
																									vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
																									vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
														vulkan_buffer->SetName(name);
													}

													auto* buffer = vulkan_buffer->Map();
													memcpy(buffer, (const void*)ubo_ptr, ubo_total_size);

													// set the offset and size as well
													// TODO: this might be a problem is the modifiers aren't matching,
													// but usually they are... std140?
													std::string_view layout_packing = glslang::TQualifier::getLayoutPackingString(glsl_type.getQualifier().layoutPacking);
													if (layout_packing != "std140")
													{
														PANIC("layout packing expected to be std140, but was {}", layout_packing);
													}

													fmt::print("{} -> SIZE {} {} {} {}\n", name, vulkan_buffer->Size(), uniform_buffer_binding_point_offset, uniform_buffer_binding_point_size, ubo_total_size);
													fmt::print("{} -> {}\n", name, uniform_block_name);
													fmt::print("{}\n", glm::to_string(glm::make_mat4((float*)current_ptr)));
													fmt::print("{}\n", glm::to_string(glm::make_mat4((float*)((char*)buffer + uniform_buffer_binding_point_offset))));

													vulkan_buffer->SetOffset(uniform_buffer_binding_point_offset);
													vulkan_buffer->SetSize(uniform_buffer_binding_point_size);

													vulkan_buffer->Unmap();
													*/

													// TODO: fix this
													// copy everything in ONE GO
													// problem: spirvm loops through members recursively only once, not many times
													// so 2d settings, etc GetMat4 has to loop through members recursively
													/*
													auto* data = reinterpret_cast<float*>(ptr);
													std::vector<float*> vm_variable = AssignOrPanic(underlying_draw_calls.GetVariableInVMRef(name));

													if (vm_variable.size() != array_size)
													{
														PANIC("Size are not equal");
													}

													for (auto i = 0; i < vm_variable.size(); i++)
													{
														(*vm_variable[i]) = data[i];
													}

													Raysterizer::Vulkan::ReflectBufferView reflect_buffer_view = AssignOrPanic(underlying_draw_calls.GetVariableInVulkanRef(name));
													auto buffer_view = reflect_buffer_view.GetBuffer();
													auto* buffer = buffer_view->Map();
													memcpy(buffer, data, array_size * size);
													*/
												}
												else if (size == sizeof(bool))
												{
													auto data = *reinterpret_cast<bool*>(ubo_ptr_offseted);
													SetVariableInVulkanAndVM(data);
												}
												else if (size == sizeof(float))
												{
													auto data = *reinterpret_cast<float*>(ubo_ptr_offseted);
													SetVariableInVulkanAndVM(data);
												}
												else if (size == sizeof(glm::vec2))
												{
													auto data = *reinterpret_cast<glm::vec2*>(ubo_ptr_offseted);
													SetVariableInVulkanAndVM(data);
												}
												else if (size == sizeof(glm::vec3))
												{
													auto data = *reinterpret_cast<glm::vec3*>(ubo_ptr_offseted);
													SetVariableInVulkanAndVM(data);
												}
												else if (size == sizeof(glm::mat4))
												{
													auto data = *reinterpret_cast<glm::mat4*>(ubo_ptr_offseted);
													SetVariableInVulkanAndVM(data);
												}
												else if (size == sizeof(glm::vec4))
												{
													auto data = *reinterpret_cast<glm::vec4*>(ubo_ptr_offseted);
													SetVariableInVulkanAndVM(data);
												}
												else
												{
													PANIC("No matching datasize");
												}
												break;
											}
										}
									}
								}
							}
						};
						SyncUniforms(vertex_analyzer, true);
						SyncUniforms(fragment_analyzer, false);
					}
				}
			}

	#define NO_DRAW 1
	#define NO_DRAW 0

			bool HandleCommonPreDraw()
			{
				auto& context = Context::Get();
				auto& state = context.state;

				if (auto vertex_shader_or_err = state.GetActiveVertexShader())
				{
					auto& vertex_shader = *vertex_shader_or_err;
					if ((std::size_t)&vertex_shader == 0)
					{
						/*
						auto current_program_id = 0;
						glGetIntegerv(GL_CURRENT_PROGRAM, &current_program_id);
						auto& program = AssignOrPanic(state.GetActiveProgram());
						PANIC("CURRENT VS ACTUAL PROGRAM IDS {} | {}", program.GetId(), current_program_id);
						*/
						return false;
					}
					if (!vertex_shader.AccessesGLPosition())
					{
						return false;
					}
				}
				else
				{
					//llvm::consumeError(vertex_shader_or_err.takeError());
					PANIC("Shader should be active!");
					return false;
				}

				if (InvalidState())
				{
					return false;
				}

				return true;
			}

			void HandleCommonPostDraw()
			{
				auto& context = Context::Get();
				auto& state = context.state;

				state.SetHasDrawnThisFrame(true);
			}

			void HOOK_CALL_CONVENTION Clear(GLbitfield mask)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(Clear, mask);

				if (init_raysterizer)
				{
					auto& context = Context::Get();
					auto& state = context.state;
					auto& pipeline_manager = context.GetPipelineManager();

					bool clear_color_enabled = mask & GL_COLOR_BUFFER_BIT;
					bool clear_depth_enabled = mask & GL_DEPTH_BUFFER_BIT;
					bool clear_stencil_enabled = mask & GL_STENCIL_BUFFER_BIT;

					auto fbo_id = AssignOrPanic(state.GetActiveFrameBufferObjectId());
					auto g_buffer_pass = AssignOrPanic(pipeline_manager.GetGBufferPass(fbo_id));
					g_buffer_pass->Clear(pipeline_manager.frame_command_buffer, clear_color_enabled, clear_depth_enabled, clear_stencil_enabled);
				}
			}

			void HOOK_CALL_CONVENTION DrawArrays(GLenum mode, GLint first, GLsizei count)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DrawArrays, mode, first, count);

				if (NO_DRAW)
				{
					return;
				}

				if (count == 0)
				{
					return;
				}

				if (!HandleCommonPreDraw())
				{
					return;
				}

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;
				auto& shader_manager = context.shader_manager;
				auto& texture_manager = context.texture_manager;

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());

				auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
				//auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());

				auto& program = AssignOrPanic(state.GetActiveProgram());

				const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

				const auto& attribs = program.GetAttribMapping();
				const auto& uniforms = program.GetUniformMapping();

				switch (mode)
				{
				case GL_TRIANGLES:
				{
					// Update reflection
					{
						SyncUniformBufferObjects();

						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
						auto& underlying_draw_calls = draw_calls;

						// Generate an ebo associated with vbo internally
						auto& vbo = AssignOrPanic(state.GetNonDivisorVertexBufferObjectBoundToVAO());
						auto& ebo = vbo.GetElementBufferObject();
						if (vbo.GetElementBufferFirst() != first || vbo.GetElementBufferCount() != count)
						{
							vbo.SetElementBufferFirst(first);
							vbo.SetElementBufferCount(count);

							// Generate indices from 0..count
							auto index_buffer_stride = underlying_draw_calls.GetIndexBufferStride();
							if (index_buffer_stride == sizeof(uint16_t))
							{
								std::vector<uint16_t> indices{};
								indices.resize(count);
								std::iota(std::begin(indices), std::end(indices), first);

								// Fake ebo
								ebo.SetPointerView(BlockHashedPointerView(indices));
							}
							else
							{
								std::vector<uint32_t> indices{};
								indices.resize(count);
								std::iota(std::begin(indices), std::end(indices), first);

								// Fake ebo
								ebo.SetPointerView(BlockHashedPointerView(indices));
							}
						}

						auto ebo_start_index = first;
						auto ebo_count = count;

						underlying_draw_calls.LoadElementBufferObject(ebo, ebo_start_index, ebo_count);

						// TODO: count is not correct here???
						auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						underlying_draw_calls.LoadVertexBufferObject(vertex_analyzer, first, count);

						underlying_draw_calls.SyncCurrentBoundTextures();
						underlying_draw_calls.PerformRasterization(vk::PrimitiveTopology::eTriangleList, first, count);

						// Calculate the MVP from the pipeline
						//auto mvp = underlying_draw_calls.CalculateModelViewProjectionFromOpenGLSource();
						underlying_draw_calls.SaveModelViewProjectionMatrixForCurrentDrawCall(vertex_analyzer);

						// Update vulkan pipeline
						underlying_draw_calls.IncrementDrawCount();
					}
					break;
				}
				case GL_TRIANGLE_STRIP:
				{
					break;
					// Update reflection
					{
						SyncUniformBufferObjects();

						auto& pipeline_manager = context.GetPipelineManager();
						auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
						auto& underlying_draw_calls = draw_calls;

						// Generate an ebo associated with vbo internally
						auto& vbo = AssignOrPanic(state.GetNonDivisorVertexBufferObjectBoundToVAO());
						auto& ebo = vbo.GetElementBufferObject();
						auto triangle_strip_count = (count - 2) * 3;
						if (vbo.GetElementBufferFirst() != first || vbo.GetElementBufferCount() != triangle_strip_count)
						{
							vbo.SetElementBufferFirst(first);
							vbo.SetElementBufferCount(count);

							// Generate indices from 0..count
							auto index_buffer_stride = underlying_draw_calls.GetIndexBufferStride();
							if (index_buffer_stride == sizeof(uint16_t))
							{
								std::vector<uint16_t> indices{};
								for (auto i = 0; i < count - 2; i++)
								{
									indices.emplace_back(i);
									indices.emplace_back(i + 1);
									indices.emplace_back(i + 2);
								}

								// Fake ebo
								ebo.SetPointerView(BlockHashedPointerView(indices));
							}
							else
							{
								std::vector<uint32_t> indices{};
								for (auto i = 0; i < count - 2; i++)
								{
									indices.emplace_back(i);
									indices.emplace_back(i + 1);
									indices.emplace_back(i + 2);
								}

								// Fake ebo
								ebo.SetPointerView(BlockHashedPointerView(indices));
							}
						}

						auto ebo_start_index = first;
						auto ebo_count = count;

						underlying_draw_calls.LoadElementBufferObject(ebo, ebo_start_index, ebo_count);

						// TODO: count is not correct here???
						auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						underlying_draw_calls.LoadVertexBufferObject(vertex_analyzer, first, count);

						underlying_draw_calls.SyncCurrentBoundTextures();
						underlying_draw_calls.PerformRasterization(vk::PrimitiveTopology::eTriangleList, first, count);

						// Calculate the MVP from the pipeline
						//auto mvp = underlying_draw_calls.CalculateModelViewProjectionFromOpenGLSource();
						underlying_draw_calls.SaveModelViewProjectionMatrixForCurrentDrawCall(vertex_analyzer);

						// Update vulkan pipeline
						underlying_draw_calls.IncrementDrawCount();
					}
					break;
				}
				default:
				{
					if (ENABLE_NOT_IMPLEMENTED_CHECK_DRAW_OPENGL)
					{
						PANIC("Other modes not supported");
					}
					break;
				}
				}

				HandleCommonPostDraw();
			}

//#define TEST_DRAW_COUNT
#ifdef TEST_DRAW_COUNT
			static auto current_draw_count = 0;
			static auto draw_count_limit = 300;
#endif

//#define TEST_DRAW_CAN_DRAW
#ifdef TEST_DRAW_CAN_DRAW
			static auto can_draw = false;
#endif

			void HOOK_CALL_CONVENTION DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DrawElements, mode, count, type, indices);

				if (NO_DRAW)
				{
					return;
				}

				if (!HandleCommonPreDraw())
				{
					return;
				}

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;
				auto& shader_manager = context.shader_manager;
				auto& texture_manager = context.texture_manager;

#ifdef TEST_DRAW_COUNT
				if (current_draw_count == draw_count_limit)
				{
					return;
				}
				current_draw_count++;
#endif

				if (auto& ebo_or_err = state.GetElementBufferObjectBoundToVAO())
				{
				}
				else
				{
					PANIC("Not implemented! Implement as direct pointer to ebo!");
				}

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				auto& ebo = AssignOrPanic(state.GetElementBufferObjectBoundToVAO());

				auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
				//auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());
				if (!vertex_shader.AccessesGLPosition())
				{
					return;
				}

				auto& program = AssignOrPanic(state.GetActiveProgram());

				const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

				// Update ebo based on type of index buffer
				auto& ebo_view = ebo.GetPointerView();
				if (!ebo.GetChangedDataStride())
				{
					switch (type)
					{
					case GL_UNSIGNED_BYTE:
					{
						ebo_view.ChangeStride(sizeof(GLubyte));
						break;
					}
					case GL_UNSIGNED_SHORT:
					{
						ebo_view.ChangeStride(sizeof(GLushort));
						break;
					}
					case GL_UNSIGNED_INT:
					{
						ebo_view.ChangeStride(sizeof(GLuint));
						break;
					}
					default:
					{
						if (ENABLE_NOT_IMPLEMENTED_CHECK_DRAW_OPENGL)
						{
							PANIC("Other modes not supported");
						}
						break;
					}
					}
				}

#ifdef TEST_DRAW_CAN_DRAW
				if (context.GetPipelineManager().GetGameType() == MiddleWare::GameType::Dolphin)
				{
					if (count == 147 || can_draw)
					{
						can_draw = true;
					}
					else
					{
						return;
					}
				}
#endif

				// Update reflection
				switch (mode)
				{
				case GL_TRIANGLES:
				{
					SyncUniformBufferObjects();

					if(0)
					{
						auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
						auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());

						auto& vertex_analyzer = vertex_shader.GetAnalyzer();
						auto& fragment_analyzer = fragment_shader.GetAnalyzer();
					
						auto& active_program = AssignOrPanic(state.GetActiveProgram());
						auto program = active_program.GetId();
						auto& shader_program = AssignOrPanic(shader_manager.GetProgram(active_program.GetId()));

						const auto& vertex_uniform_blocks = vertex_analyzer.GetUniformBlocks();
						for (auto& [name, uniform] : vertex_analyzer.GetUniforms())
						{
							auto& object_reflection = uniform.GetObjectReflection();
							const auto& glsl_type = *uniform.GetGLSLType();
							auto offset = object_reflection.offset;
							auto size = Raysterizer::OpenGL::Util::GLenumToSize(uniform.GetGLType());
							auto array_size = 0;

							if (uniform.IsPartOfUniformBlock())
							{
								auto& uniform_block_name = uniform.GetParentUniformBlockName();
								if (auto found = vertex_uniform_blocks.find(uniform_block_name); found != std::end(vertex_uniform_blocks))
								{
									const auto& [_, vertex_uniform_block] = *found;
									for (auto& [index, uniform_block] : shader_program.GetUniformBlockMapping())
									{
										if (uniform_block_name == uniform_block.name)
										{
											auto& uniform_buffer_binding_point = buffer_manager.GetUniformBufferBindingPoint(uniform_block.block_binding);
											auto uniform_buffer_binding_point_offset = uniform_buffer_binding_point.GetBoundUBOIDOffset();
											auto uniform_buffer_binding_point_size = uniform_buffer_binding_point.GetBoundUBOIDSize();

											auto& ubo = AssignOrPanic(buffer_manager.GetUBO(uniform_buffer_binding_point.GetBoundUBOID()));

											GLint o_id{};
											{

												GLint max_length{};
												CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
												std::string name((max_length + 1), 0);

												GLint uniform_count{};
												CallOriginalOpenGL(GetProgramiv, program, GL_ACTIVE_UNIFORMS, &uniform_count);

												for (GLuint i = 0; i < uniform_count; i++)
												{
													GLint size{};
													GLenum type{};

													//TODO: replace name with glsl parser
													auto* name_str = reinterpret_cast<GLchar*>(name.data());
													CallOriginalOpenGL(GetActiveUniform, program, i, max_length, nullptr, &size, &type, name_str);

													auto uniform_location = CallOriginalOpenGL(GetUniformLocation, program, name_str);

													// CB0[0] -> CB0
													std::string_view name_view{ name_str };
													if (auto found = name_view.find("["); found != std::string_view::npos)
													{
														name_view = name_view.substr(0, found);
													}


													fmt::print("{} \n", name_view);
												}
											}
											/*
											CallOriginalOpenGL(BindBuffer, GL_UNIFORM_BUFFER, ubo.GetId());
											auto ptr = CallOriginalOpenGL(MapBuffer, GL_UNIFORM_BUFFER, GL_READ_WRITE);
										
											auto& ubo_ptr_view = ubo.GetPointerView();
											auto ubo_total_size = ubo_ptr_view.GetTotalSize();
											auto ubo_ptr = ubo_ptr_view.GetDataAs<uintptr_t>();
											auto gg_ubo_ptr = ubo_ptr + offset;

											auto ubo_ptr_offseted = gg_ubo_ptr + uniform_buffer_binding_point_offset;

											for (auto i = 0; i < uniform_buffer_binding_point_size; i++)
											{
												auto a1 = ((uint8_t*)(ubo_ptr)) + i;
												auto b2 = ((uint8_t*)(ptr)) + i;

												if (*a1 != *b2)
												{
													PANIC("NOT SAME!");
												}
											}

											CallOriginalOpenGL(BindBuffer, GL_UNIFORM_BUFFER, o_id);
											*/
										}
									}
								}
							}
						}
					}

					auto& pipeline_manager = context.GetPipelineManager();
					auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
					auto& underlying_draw_calls = draw_calls;

					std::size_t ebo_pointer_offset = (std::size_t)indices;
					auto ebo_start_index = ebo_pointer_offset / ebo_view.GetStride();
					//auto ebo_count = ebo_view.GetNumElements() - ebo_start_index;
					auto ebo_count = count;

					underlying_draw_calls.LoadElementBufferObject(ebo, ebo_start_index, ebo_count);

					// TODO: count is not correct here???
					//vulkan_pipeline.LoadVertexBufferObject(vbo, vertex_attrib_pointers, 0, count);
					auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
					auto& vertex_analyzer = vertex_shader.GetAnalyzer();
					underlying_draw_calls.LoadVertexBufferObject(vertex_analyzer, 0, count);

					underlying_draw_calls.SyncCurrentBoundTextures();
					underlying_draw_calls.PerformRasterization(vk::PrimitiveTopology::eTriangleList, 0, count);

					// Calculate the MVP from the pipeline
					//auto mvp = underlying_draw_calls.CalculateModelViewProjectionFromOpenGLSource();
					underlying_draw_calls.SaveModelViewProjectionMatrixForCurrentDrawCall(vertex_analyzer);

					// Update vulkan pipeline
					underlying_draw_calls.IncrementDrawCount();
					break;
				}
				default:
				{
					if (ENABLE_NOT_IMPLEMENTED_CHECK_DRAW_OPENGL)
					{
						PANIC("Other modes not supported");
					}
					break;
				}
				}

				HandleCommonPostDraw();
			}

			void HOOK_CALL_CONVENTION DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DrawElementsBaseVertex, mode, count, type, indices, basevertex);

				if (NO_DRAW)
				{
					return;
				}

				if (!HandleCommonPreDraw())
				{
					return;
				}

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;
				auto& shader_manager = context.shader_manager;
				auto& texture_manager = context.texture_manager;

				if (auto& ebo_or_err = state.GetElementBufferObjectBoundToVAO())
				{
				}
				else
				{
					PANIC("Not implemented! Implement as direct pointer to ebo!");
				}

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				auto& ebo = AssignOrPanic(state.GetElementBufferObjectBoundToVAO());

				auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
				//auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());
				if (!vertex_shader.AccessesGLPosition())
				{
					return;
				}

				auto& program = AssignOrPanic(state.GetActiveProgram());

				const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

				// Update ebo based on type of index buffer
				auto& ebo_view = ebo.GetPointerView();
				if (!ebo.GetChangedDataStride())
				{
					switch (type)
					{
					case GL_UNSIGNED_BYTE:
					{
						ebo_view.ChangeStride(sizeof(GLubyte));
						break;
					}
					case GL_UNSIGNED_SHORT:
					{
						ebo_view.ChangeStride(sizeof(GLushort));
						break;
					}
					case GL_UNSIGNED_INT:
					{
						ebo_view.ChangeStride(sizeof(GLuint));
						break;
					}
					default:
					{
						if (ENABLE_NOT_IMPLEMENTED_CHECK_DRAW_OPENGL)
						{
							PANIC("Other modes not supported");
						}
						break;
					}
					}
				}

				// Update reflection
				switch (mode)
				{
				case GL_TRIANGLES:
				{
					SyncUniformBufferObjects();

					auto& pipeline_manager = context.GetPipelineManager();
					auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
					auto& underlying_draw_calls = draw_calls;

					std::size_t ebo_pointer_offset = (std::size_t)indices;
					auto ebo_start_index = ebo_pointer_offset / ebo_view.GetStride();
					//auto ebo_count = ebo_view.GetNumElements() - ebo_start_index;
					auto ebo_count = count;

					underlying_draw_calls.LoadElementBufferObject(ebo, ebo_start_index, ebo_count);

					// TODO: count is not correct here???
					//vulkan_pipeline.LoadVertexBufferObject(vbo, vertex_attrib_pointers, 0, count);
					auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
					auto& vertex_analyzer = vertex_shader.GetAnalyzer();
					underlying_draw_calls.LoadVertexBufferObject(vertex_analyzer, basevertex, count);

					underlying_draw_calls.SyncCurrentBoundTextures();
					underlying_draw_calls.PerformRasterization(vk::PrimitiveTopology::eTriangleList, basevertex, count);

					// Calculate the MVP from the pipeline
					//auto mvp = underlying_draw_calls.CalculateModelViewProjectionFromOpenGLSource();
					underlying_draw_calls.SaveModelViewProjectionMatrixForCurrentDrawCall(vertex_analyzer);

					// Update vulkan pipeline
					underlying_draw_calls.IncrementDrawCount();
					break;
				}
				default:
				{
					if (ENABLE_NOT_IMPLEMENTED_CHECK_DRAW_OPENGL)
					{
						PANIC("Other modes not supported");
					}
					break;
				}
				}

				HandleCommonPostDraw();
			}

			void HOOK_CALL_CONVENTION DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instancecount)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DrawElementsInstanced, mode, count, type, indices, instancecount);

				if (0 && NO_DRAW)
				{
					return;
				}

				if (!HandleCommonPreDraw())
				{
					return;
				}

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;
				auto& shader_manager = context.shader_manager;
				auto& texture_manager = context.texture_manager;
				auto& pipeline_manager = context.GetPipelineManager();

				if (auto& ebo_or_err = state.GetElementBufferObjectBoundToVAO())
				{
				}
				else
				{
					PANIC("Not implemented! Implement as direct pointer to ebo!");
				}

				auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				auto& ebo = AssignOrPanic(state.GetElementBufferObjectBoundToVAO());

				auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
				auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());

				//fmt::print("{}\n", vertex_shader.GetSource());
				//fmt::print("{}\n", fragment_shader.GetSource());

				auto& program = AssignOrPanic(state.GetActiveProgram());

				const auto& vertex_attrib_pointers = vao.GetVertexAttribPointers();

				// Update ebo based on type of index buffer
				auto& ebo_view = ebo.GetPointerView();
				if (0 && pipeline_manager.GetGameType() == MiddleWare::GameType::Roblox)
				{
					if (!ebo_view && count == 36 && type == GL_UNSIGNED_SHORT)
					{
						std::vector<uint16_t> data{
							0,
							1,
							2,
							2,
							1,
							3,
							4,
							5,
							6,
							6,
							5,
							7,
							8,
							9,
							10,
							10,
							9,
							11,
							12,
							13,
							14,
							14,
							13,
							15,
							16,
							17,
							18,
							18,
							17,
							19,
							20,
							21,
							22,
							22,
							21,
							23,
						};
						ebo.SetPointerView(BlockHashedPointerView(data));
					}
					if (!ebo_view)
					{
						return;
					}
				}
				if (!ebo_view)
				{
					PANIC("EBO not valid!");
				}

				if (!ebo.GetChangedDataStride())
				{
					switch (type)
					{
					case GL_UNSIGNED_BYTE:
					{
						ebo_view.ChangeStride(sizeof(GLubyte));
						break;
					}
					case GL_UNSIGNED_SHORT:
					{
						ebo_view.ChangeStride(sizeof(GLushort));
						break;
					}
					case GL_UNSIGNED_INT:
					{
						ebo_view.ChangeStride(sizeof(GLuint));
						break;
					}
					default:
					{
						PANIC("Not possible");
						break;
					}
					}
				}

				// Update reflection
				switch (mode)
				{
				case GL_TRIANGLES:
				{
					SyncUniformBufferObjects();

					auto& pipeline_manager = context.GetPipelineManager();
					auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
					auto& underlying_draw_calls = draw_calls;

					std::size_t ebo_pointer_offset = (std::size_t)indices;
					auto ebo_start_index = ebo_pointer_offset / ebo_view.GetStride();
					//auto ebo_count = ebo_view.GetNumElements() - ebo_start_index;
					auto ebo_count = count;

					underlying_draw_calls.LoadElementBufferObject(ebo, ebo_start_index, ebo_count);
					
					// TODO: count is not correct here???
					//vulkan_pipeline.LoadVertexBufferObject(vbo, vertex_attrib_pointers, 0, count);
					auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
					auto& vertex_analyzer = vertex_shader.GetAnalyzer();
					underlying_draw_calls.LoadVertexBufferObject(vertex_analyzer, 0, count);

					underlying_draw_calls.SyncCurrentBoundTextures();
					underlying_draw_calls.PerformRasterization(vk::PrimitiveTopology::eTriangleList, 0, count, instancecount);

					//fmt::print("{}\n", underlying_draw_calls.GetRaytracingGLSL());

					// Calculate the MVP from the pipeline
					//auto mvp = underlying_draw_calls.CalculateModelViewProjectionFromOpenGLSource();
					underlying_draw_calls.SaveModelViewProjectionMatrixForCurrentDrawCall(vertex_analyzer, instancecount);

					// Update vulkan pipeline
					underlying_draw_calls.IncrementDrawCount();
					break;
				}
				default:
				{
					PANIC("Other modes not supported");
					break;
				}
				}

				HandleCommonPostDraw();
			}

			void HOOK_CALL_CONVENTION Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(Viewport, x, y, width, height);

				auto& context = Context::Get();
				auto& state = context.state;

				state.SetViewport(x, y, width, height);
			}

			void HOOK_CALL_CONVENTION ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(ClearColor, red, green, blue, alpha);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				state.SetClearColor(glm::vec4{ red, green, blue, alpha });
				render_state.SetClearColor({ red, green, blue, alpha });
			}
			
			void HOOK_CALL_CONVENTION ClearDepth(GLclampf depth)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(ClearDepth, depth);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				render_state.SetClearDepth(depth);
			}

			void HOOK_CALL_CONVENTION DepthFunc(GLenum func)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DepthFunc, func);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				auto depth_func = OpenGL::Util::GlCompareFuncToVkCompareOp(func);
				render_state.SetDepthFunc(depth_func);
			}

			void HOOK_CALL_CONVENTION DepthRange(GLdouble nearVal, GLdouble farVal)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(DepthRange, nearVal, farVal);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				render_state.SetDepthRange(nearVal, farVal);
			}

			void HOOK_CALL_CONVENTION BlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BlendColor, red, green, blue, alpha);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				render_state.SetBlendColor({ red, green, blue, alpha });
			}

			void HOOK_CALL_CONVENTION BlendEquation(GLenum mode)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BlendEquation, mode);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				auto blend_op = OpenGL::Util::GlBlendEquationToVkBlendOp(mode);

				render_state.SetColorBlendOp(blend_op);
				render_state.SetAlphaBlendOp(blend_op);
			}

			void HOOK_CALL_CONVENTION BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BlendEquationSeparate, modeRGB, modeAlpha);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				auto color_blend_op = OpenGL::Util::GlBlendEquationToVkBlendOp(modeRGB);
				auto alpha_blend_op = OpenGL::Util::GlBlendEquationToVkBlendOp(modeAlpha);

				render_state.SetColorBlendOp(color_blend_op);
				render_state.SetAlphaBlendOp(alpha_blend_op);
			}

			void HOOK_CALL_CONVENTION BlendFunc(GLenum sfactor, GLenum dfactor)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BlendFunc, sfactor, dfactor);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				auto src_blend_factor = OpenGL::Util::GlBlendFactorToVkBlendFactor(sfactor);
				auto dst_blend_factor = OpenGL::Util::GlBlendFactorToVkBlendFactor(dfactor);

				render_state.SetSrcColorBlendFactor(src_blend_factor);
				render_state.SetDstColorBlendFactor(dst_blend_factor);
				render_state.SetSrcAlphaBlendFactor(src_blend_factor);
				render_state.SetDstAlphaBlendFactor(dst_blend_factor);
			}

			void HOOK_CALL_CONVENTION BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(BlendFuncSeparate, srcRGB, dstRGB, srcAlpha, dstAlpha);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				auto src_color_blend_factor = OpenGL::Util::GlBlendFactorToVkBlendFactor(srcRGB);
				auto dst_color_blend_factor = OpenGL::Util::GlBlendFactorToVkBlendFactor(dstRGB);
				auto src_alpha_blend_factor = OpenGL::Util::GlBlendFactorToVkBlendFactor(srcAlpha);
				auto dst_alpha_blend_factor = OpenGL::Util::GlBlendFactorToVkBlendFactor(dstAlpha);

				render_state.SetSrcColorBlendFactor(src_color_blend_factor);
				render_state.SetDstColorBlendFactor(dst_color_blend_factor);
				render_state.SetSrcAlphaBlendFactor(src_alpha_blend_factor);
				render_state.SetDstAlphaBlendFactor(dst_alpha_blend_factor);
			}

			void HOOK_CALL_CONVENTION ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(ColorMask, red, green, blue, alpha);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				vk::ColorComponentFlags color_component_flags{};
				if (red)
				{
					color_component_flags |= vk::ColorComponentFlagBits::eR;
				}
				if (green)
				{
					color_component_flags |= vk::ColorComponentFlagBits::eG;
				}
				if (blue)
				{
					color_component_flags |= vk::ColorComponentFlagBits::eB;
				}
				if (alpha)
				{
					color_component_flags |= vk::ColorComponentFlagBits::eA;
				}

				render_state.SetColorWriteMask(color_component_flags);
			}

			void HOOK_CALL_CONVENTION LogicOp(GLenum opcode)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				CallOriginalOpenGL(LogicOp, opcode);

				auto& context = Context::Get();
				auto& state = context.state;
				auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
				auto& render_state = raysterizer_vulkan_state.GetRenderState();

				render_state.SetColorLogicOp(OpenGL::Util::GlLogicOpToVkLogicOp(opcode));
			}

			namespace
			{
				void HOOK_CALL_CONVENTION Disable(GLenum cap)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Disable, cap);

					bool set_value = false;

					auto& context = Context::Get();
					auto& state = context.state;
					auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
					auto& render_state = raysterizer_vulkan_state.GetRenderState();

					switch (cap)
					{
						case GL_DEPTH_TEST:
						{
							render_state.SetDepthTest(set_value);
							break;
						}
						case GL_CULL_FACE:
						{
							render_state.SetCullFace(set_value);
							break;
						}
						case GL_BLEND:
						{
							render_state.SetBlend(set_value);
							break;
						}
						case GL_COLOR_LOGIC_OP:
						{
							render_state.SetColorLogicOpEnabled(set_value);
							break;
						}
						default:
						{
							break;
						}
					}
				}

				void HOOK_CALL_CONVENTION Enable(GLenum cap)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(Enable, cap);

					bool set_value = true;

					auto& context = Context::Get();
					auto& state = context.state;
					auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
					auto& render_state = raysterizer_vulkan_state.GetRenderState();

					switch (cap)
					{
						case GL_DEPTH_TEST:
						{
							render_state.SetDepthTest(set_value);
							break;
						}
						case GL_CULL_FACE:
						{
							render_state.SetCullFace(set_value);
							break;
						}
						case GL_BLEND:
						{
							render_state.SetBlend(set_value);
							break;
						}
						case GL_COLOR_LOGIC_OP:
						{
							render_state.SetColorLogicOpEnabled(set_value);
							break;
						}
						default:
						{
							break;
						}
					}
				}

				void HOOK_CALL_CONVENTION DepthMask(GLboolean flag)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(DepthMask, flag);

					auto& context = Context::Get();
					auto& state = context.state;
					auto& raysterizer_vulkan_state = context.raysterizer_vulkan_state;
					auto& render_state = raysterizer_vulkan_state.GetRenderState();

					render_state.SetDepthWriting(flag);
				}

				const GLubyte* HOOK_CALL_CONVENTION GetString(GLenum name)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto result = CallOriginalOpenGL(GetString, name);

					if (result)
					{
						if (name == GL_EXTENSIONS)
						{
							const static std::vector<std::string> ignored_extensions = Config["raysterizer"]["opengl"]["ignored_extensions"];

							auto extension_str = std::string(reinterpret_cast<const char*>(result));
							auto extensions = RaysterizerEngine::Util::SplitString(extension_str, " ");

							for (auto& extension : extensions)
							{
								if (std::find(std::begin(ignored_extensions), std::end(ignored_extensions), extension) != std::end(ignored_extensions))
								{
									extension = "";
								}
							}

							static std::string output_extensions;
							output_extensions = RaysterizerEngine::Util::JoinStrings(extensions, " ");
							return reinterpret_cast<const GLubyte*>(output_extensions.c_str());
						}
						else if (name == GL_VERSION)
						{
							const static std::string opengl_version = Config["raysterizer"]["opengl"]["version"];
							return reinterpret_cast<const GLubyte*>(opengl_version.c_str());
						}
					}

					return result;
				}

				const GLubyte* HOOK_CALL_CONVENTION GetStringi(GLenum name, GLuint index)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					auto result = CallOriginalOpenGL(GetStringi, name, index);

					if (result)
					{
						if (name == GL_EXTENSIONS)
						{
							const static std::vector<std::string> ignored_extensions = Config["raysterizer"]["opengl"]["ignored_extensions"];

							auto extension = std::string(reinterpret_cast<const char*>(result));

							if (std::find(std::begin(ignored_extensions), std::end(ignored_extensions), extension) != std::end(ignored_extensions))
							{
								extension = "";
							}

							static std::vector<std::string> output_extensions;
							if (output_extensions.empty())
							{
								GLint num_extensions{};
								glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
								output_extensions.resize(num_extensions);
							}
							output_extensions[index] = extension;
							return reinterpret_cast<const GLubyte*>(output_extensions[index].c_str());
						}
					}

					return result;
				}

				void HOOK_CALL_CONVENTION GetIntegerv(GLenum pname, GLint* data)
				{
					ScopedCPUProfileOpenGLCurrentFunction();

					CallOriginalOpenGL(GetIntegerv, pname, data);

					const static std::string opengl_version = Config["raysterizer"]["opengl"]["version"];

					auto versions = RaysterizerEngine::Util::SplitString(opengl_version, ".");
					auto major = std::stoi(versions[0]);
					auto middle = std::stoi(versions[1]);
					auto minor = std::stoi(versions[2]);

					switch (pname)
					{
					case GL_MAJOR_VERSION:
					{
						*data = major;
						break;
					}
					case GL_MINOR_VERSION:
					{
						*data = middle;
						break;
					}
					default:
					{
						break;
					}
					}
					
				}
			};

			BOOL HOOK_CALL_CONVENTION hook_wglSwapBuffers(HDC hdc)
			{
				ScopedCPUProfileOpenGLCurrentFunction();

				auto result = CallOriginalOpenGL(hook_wglSwapBuffers, hdc);

				InitializeRaysterizer();

				if (0 && NO_DRAW)
				{
					return result;
				}

#ifdef TEST_DRAW_COUNT
				current_draw_count = 0;
#endif
#ifdef TEST_DRAW_CAN_DRAW
				can_draw = false;
#endif

				auto& context = Context::Get();
				auto& state = context.state;
				auto& buffer_manager = context.buffer_manager;
				auto& shader_manager = context.shader_manager;
				auto& texture_manager = context.texture_manager;
				auto& pipeline_manager = context.GetPipelineManager();

				// There's nothing to draw!
				if (!state.GetHasDrawnThisFrame())
				{
					return result;
				}

				if (auto program_or_err = state.GetActiveProgram())
				{
					auto& program = *program_or_err;

					auto& draw_calls = AssignOrPanic(pipeline_manager.GetProgramToDrawCalls(program.GetId()));
					auto& underlying_draw_calls = draw_calls;
				}
				else
				{
					llvm::consumeError(program_or_err.takeError());
					//PANIC("No active program!");
				}

				// Has opengl been called yet? Check vbo if it has been attached (I mean, you need a vertex buffer if you're drawing anything)
				/*
				if (auto vao_or_err = state.GetActiveVertexArrayObject())
				{

				}
				else
				{
					llvm::consumeError(vao_or_err.takeError());
					return result;
				}
				*/

				//auto& vao = AssignOrPanic(state.GetActiveVertexArrayObject());
				//auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
				//auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());

				//auto& vertex_shader = AssignOrPanic(state.GetActiveVertexShader());
				//auto& fragment_shader = AssignOrPanic(state.GetActiveFragmentShader());
				//auto& program = AssignOrPanic(state.GetActiveProgram());

				//const auto& attribs = program.GetAttribMapping();
				//const auto& uniforms = program.GetUniformMapping();

				DrawFrameRaysterizer();

				// Clear buffers after they are used
				for (auto& [_, program] : shader_manager.GetPrograms())
				{
					if (auto draw_calls_or_err = pipeline_manager.GetProgramToDrawCalls(program.GetId()))
					{
						auto& draw_calls = *draw_calls_or_err;

						auto& underlying_draw_calls = draw_calls;
						underlying_draw_calls.ResetDrawCount();
					}
					else
					{
						llvm::consumeError(draw_calls_or_err.takeError());
					}
				}

				{
					auto marked_delete_buffers = state.GetAndFlushMarkedDeleteBuffers();
					CallOriginalOpenGL(DeleteBuffers, marked_delete_buffers.size(), marked_delete_buffers.data());

					// assume buffer object for now unless bind buffer
					for (const auto& id : marked_delete_buffers)
					{
						if (RAYSTERIZER_OPENGL_INTERLOP_BUFFER_OPTIMIZATION)
						{
							// ignore...
							if (auto err = context.buffer_manager.DeleteBuffer(id))
							{
								llvm::consumeError(std::move(err));
							}
						}
						else
						{
							PanicIfError(context.buffer_manager.DeleteBuffer(id));
						}
					}
				}

				state.SetHasDrawnThisFrame(false);

				return result;
			}
		}

		Context::Context()
		{
			/*
			std::thread([&]()
				{
					Init();
				}).detach();
			*/
			Init();
		}

		namespace Proxy
		{
			namespace
			{
				void HOOK_CALL_CONVENTION GenVertexArrays(GLsizei n, GLuint* buffers)
				{
					static GLuint index = 1;
					for (auto i = 0; i < n; i++)
					{
						buffers[i] = index++;
					}
				}

				void HOOK_CALL_CONVENTION GenBuffers(GLsizei n, GLuint* buffers)
				{
					static GLuint index = 1;
					for (auto i = 0; i < n; i++)
					{
						buffers[i] = index++;
					}
				}

				void HOOK_CALL_CONVENTION DeleteBuffers(GLsizei n, const GLuint* buffers)
				{
				}

				void HOOK_CALL_CONVENTION DeleteVertexArrays(GLsizei n, const GLuint* buffers)
				{
				}

				void HOOK_CALL_CONVENTION BindBuffer(GLenum target, GLuint buffer)
				{
				}

				void HOOK_CALL_CONVENTION BindVertexArray(GLuint array)
				{
				}

				void HOOK_CALL_CONVENTION BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
				{
				}

				void HOOK_CALL_CONVENTION BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
				{
				}

				void HOOK_CALL_CONVENTION VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
				{
				}

				void HOOK_CALL_CONVENTION VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
				{
				}

				void HOOK_CALL_CONVENTION VertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void* pointer)
				{
				}

				void HOOK_CALL_CONVENTION EnableVertexAttribArray(GLuint index)
				{
				}

				void HOOK_CALL_CONVENTION DisableVertexAttribArray(GLuint index)
				{
				}

				void HOOK_CALL_CONVENTION VertexAttribDivisor(GLuint index, GLuint divisor)
				{
				}
			}

			namespace
			{
				GLuint HOOK_CALL_CONVENTION CreateShader(GLenum shaderType)
				{
					/*
					static GLuint index = 1;
					return index++;
					*/
					return CallOriginalOpenGLActual(OpenGLRedirection::CreateShader, shaderType);
				}

				void HOOK_CALL_CONVENTION DeleteShader(GLuint shader)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::DeleteShader, shader);
				}

				void HOOK_CALL_CONVENTION ShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::ShaderSource, shader, count, string, length);
				}

				void HOOK_CALL_CONVENTION ShaderBinary(GLsizei count, const GLuint* shaders, GLenum binaryFormat, const void* binary, GLsizei length)
				{
				}

				void HOOK_CALL_CONVENTION CompileShader(GLuint shader)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::CompileShader, shader);
				}

				void HOOK_CALL_CONVENTION GetShaderiv(GLuint shader, GLenum pname, GLint* params)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::GetShaderiv, shader, pname, params);
				}

				GLuint HOOK_CALL_CONVENTION CreateProgram()
				{
					//static GLuint index = 1;
					//return index++;
					return CallOriginalOpenGLActual(OpenGLRedirection::CreateProgram);
				}

				void HOOK_CALL_CONVENTION DeleteProgram(GLuint program_id)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::DeleteProgram, program_id);
				}

				void HOOK_CALL_CONVENTION AttachShader(GLuint program, GLuint shader)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::AttachShader, program, shader);
				}

				void HOOK_CALL_CONVENTION LinkProgram(GLuint program)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::LinkProgram, program);
				}

				void HOOK_CALL_CONVENTION UseProgram(GLuint program)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::UseProgram, program);
				}

				void HOOK_CALL_CONVENTION GetProgramiv(GLuint program, GLenum pname, GLint* params)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::GetProgramiv, program, pname, params);
				}

				void HOOK_CALL_CONVENTION GetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::GetActiveUniform, program, index, bufSize, length, size, type, name);
				}

				GLint HOOK_CALL_CONVENTION GetUniformLocation(GLuint program, const GLchar* name)
				{
					return CallOriginalOpenGLActual(OpenGLRedirection::GetUniformLocation, program, name);
				}

				void HOOK_CALL_CONVENTION GetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::GetActiveAttrib, program, index, bufSize, length, size, type, name);
				}

				GLint HOOK_CALL_CONVENTION GetAttribLocation(GLuint program, const GLchar* name)
				{
					return CallOriginalOpenGLActual(OpenGLRedirection::GetAttribLocation, program, name);
				}

				GLuint HOOK_CALL_CONVENTION GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
				{
					return CallOriginalOpenGLActual(OpenGLRedirection::GetUniformBlockIndex, program, uniformBlockName);
				}

				void HOOK_CALL_CONVENTION UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
				{
					CallOriginalOpenGLActual(OpenGLRedirection::UniformBlockBinding, program, uniformBlockIndex, uniformBlockBinding);
				}
			}

			namespace
			{
				void HOOK_CALL_CONVENTION GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
				{
				}

				void HOOK_CALL_CONVENTION GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
				{
				}

				void HOOK_CALL_CONVENTION BindBufferBase(GLenum target, GLuint index, GLuint buffer)
				{
				}

				void HOOK_CALL_CONVENTION BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
				{
				}

				Expected<BufferObjectContainingData&> GetBufferFromTarget(GLenum target)
				{
					auto& context = Context::Get();
					auto& state = context.state;

					if (target == GL_ARRAY_BUFFER)
					{
						auto& vbo = AssignOrPanic(state.GetActiveVertexBufferObject());
						return vbo;
					}
					else if (target == GL_ELEMENT_ARRAY_BUFFER)
					{
						auto& ebo = AssignOrPanic(state.GetActiveElementBufferObject());
						return ebo;
					}
					else if (target == GL_UNIFORM_BUFFER)
					{
						auto& ubo = AssignOrPanic(state.GetActiveUniformBufferObject());
						return ubo;
					}
					else
					{
						return StringError("Target not supported: {}", target);
					}
				}

				void* HOOK_CALL_CONVENTION MapBuffer(GLenum target, GLenum access)
				{
					auto& buffer = AssignOrPanic(GetBufferFromTarget(target));
					void* mapped_buffer = buffer.GetData().data();
					if (mapped_buffer == nullptr)
					{
						PANIC("Data mapped is null with target: {} access: {}", target, access);
					}

					return mapped_buffer;
				}

				void HOOK_CALL_CONVENTION UnmapBuffer(GLenum target)
				{
					auto& buffer = AssignOrPanic(GetBufferFromTarget(target));
				}

				void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
				{
					auto& buffer = AssignOrPanic(GetBufferFromTarget(target));
					auto* mapped_buffer = (uint8_t*)buffer.GetData().data();
					if (mapped_buffer == nullptr)
					{
						PANIC("Data mapped is null with target: {} access: {}", target, access);
					}

					if (offset + length > buffer.GetData().size())
					{
						PANIC("Out of bounds {} > {}", offset + length, buffer.GetData().size());
					}

					auto mapped_buffer_offset = mapped_buffer + offset;
					return mapped_buffer_offset;
				}

				void HOOK_CALL_CONVENTION Uniform1f(GLint location, GLfloat v0)
				{
				}

				void HOOK_CALL_CONVENTION Uniform2f(GLint location, GLfloat v0, GLfloat v1)
				{
				}

				void HOOK_CALL_CONVENTION Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
				{
				}

				void HOOK_CALL_CONVENTION Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
				{
				}

				void HOOK_CALL_CONVENTION Uniform1i(GLint location, GLint v0)
				{
				}

				void HOOK_CALL_CONVENTION Uniform2i(GLint location, GLint v0, GLint v1)
				{
				}

				void HOOK_CALL_CONVENTION Uniform3i(GLint location, GLint v0, GLint v1, GLint v2)
				{
				}

				void HOOK_CALL_CONVENTION Uniform4i(GLint location, GLuint v0, GLint v1, GLint v2, GLint v3)
				{
				}

				void HOOK_CALL_CONVENTION Uniform1ui(GLint location, GLuint v0)
				{
				}

				void HOOK_CALL_CONVENTION Uniform2ui(GLint location, GLuint v0, GLuint v1)
				{
				}

				void HOOK_CALL_CONVENTION Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
				{
				}

				void HOOK_CALL_CONVENTION Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
				{
				}

				void HOOK_CALL_CONVENTION Uniform1fv(GLint location, GLsizei count, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform2fv(GLint location, GLsizei count, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform3fv(GLint location, GLsizei count, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform4fv(GLint location, GLsizei count, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform1iv(GLint location, GLsizei count, const GLint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform2iv(GLint location, GLsizei count, const GLint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform3iv(GLint location, GLsizei count, const GLint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform4iv(GLint location, GLsizei count, const GLint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform1uiv(GLint location, GLsizei count, const GLuint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform2uiv(GLint location, GLsizei count, const GLuint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform3uiv(GLint location, GLsizei count, const GLuint* value)
				{
				}

				void HOOK_CALL_CONVENTION Uniform4uiv(GLint location, GLsizei count, const GLuint* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}

				void HOOK_CALL_CONVENTION UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
				{
				}
			}

			namespace
			{
				void HOOK_CALL_CONVENTION GenSamplers(GLsizei n, GLuint* samplers)
				{
					static GLuint index = 1;
					for (auto i = 0; i < n; i++)
					{
						samplers[i] = index++;
					}
				}

				void HOOK_CALL_CONVENTION DeleteSamplers(GLsizei n, const GLuint* samplers)
				{
				}

				void HOOK_CALL_CONVENTION BindSampler(GLenum target, GLuint sampler)
				{
				}

				void HOOK_CALL_CONVENTION SamplerParameteri(GLuint sampler, GLenum pname, GLint param)
				{

				}

				void HOOK_CALL_CONVENTION GenTextures(GLsizei n, GLuint* textures)
				{
					static GLuint index = 1;
					for (auto i = 0; i < n; i++)
					{
						textures[i] = index++;
					}
				}

				void HOOK_CALL_CONVENTION DeleteTextures(GLsizei n, const GLuint* textures)
				{
				}

				void HOOK_CALL_CONVENTION ActiveTexture(GLenum texture)
				{
				}

				void HOOK_CALL_CONVENTION BindTexture(GLenum target, GLuint texture)
				{
				}

				void HOOK_CALL_CONVENTION TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
				{
				}

				void HOOK_CALL_CONVENTION TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
				{
				}
				
				void HOOK_CALL_CONVENTION TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
				{
				}

				void HOOK_CALL_CONVENTION TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels)
				{
				}

				void HOOK_CALL_CONVENTION TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data)
				{
				}

				void HOOK_CALL_CONVENTION TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* data)
				{
				}

				void HOOK_CALL_CONVENTION GenerateMipmap(GLenum target)
				{
				}

				void HOOK_CALL_CONVENTION TexParameteri(GLenum target, GLenum pname, GLint param)
				{
				}

				void HOOK_CALL_CONVENTION GenFramebuffers(GLsizei n, GLuint* ids)
				{
				}

				void HOOK_CALL_CONVENTION DeleteFramebuffers(GLsizei n, const GLuint* ids)
				{
				}

				void HOOK_CALL_CONVENTION FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
				{
				}

				void HOOK_CALL_CONVENTION FramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
				{
					FramebufferTextureLayer(target, attachment, texture, level, 0);
				}

				void HOOK_CALL_CONVENTION BindFramebuffer(GLenum target, GLuint framebuffer)
				{
				}
			}

			namespace
			{
				void HOOK_CALL_CONVENTION Clear(GLbitfield mask)
				{
				}

				void HOOK_CALL_CONVENTION DrawArrays(GLenum mode, GLint first, GLsizei count)
				{
				}

				void HOOK_CALL_CONVENTION DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
				{
				}

				void HOOK_CALL_CONVENTION DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint basevertex)
				{
				}

				void HOOK_CALL_CONVENTION DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instancecount)
				{
				}

				void HOOK_CALL_CONVENTION Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
				{
				}

				void HOOK_CALL_CONVENTION ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
				{
				}

				void HOOK_CALL_CONVENTION ClearDepth(GLclampf depth)
				{
				}

				void HOOK_CALL_CONVENTION DepthFunc(GLenum func)
				{
				}

				void HOOK_CALL_CONVENTION DepthRange(GLdouble nearVal, GLdouble farVal)
				{
				}

				void HOOK_CALL_CONVENTION BlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
				{
				}

				void HOOK_CALL_CONVENTION BlendEquation(GLenum mode)
				{
				}

				void HOOK_CALL_CONVENTION BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
				{
				}

				void HOOK_CALL_CONVENTION BlendFunc(GLenum sfactor, GLenum dfactor)
				{
				}

				void HOOK_CALL_CONVENTION BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
				{
				}

				void HOOK_CALL_CONVENTION ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
				{
				}

				void HOOK_CALL_CONVENTION LogicOp(GLenum opcode)
				{
				}
			}

			namespace
			{
				void HOOK_CALL_CONVENTION Disable(GLenum cap)
				{
				}

				void HOOK_CALL_CONVENTION Enable(GLenum cap)
				{
				}

				void HOOK_CALL_CONVENTION DepthMask(GLboolean flag)
				{
				}

				const GLubyte* HOOK_CALL_CONVENTION GetString(GLenum name)
				{
					return nullptr;
				}

				const GLubyte* HOOK_CALL_CONVENTION GetStringi(GLenum name, GLuint index)
				{
					return nullptr;
				}

				void HOOK_CALL_CONVENTION GetIntegerv(GLenum pname, GLint* data)
				{

				}
			}
		}

		void Context::Init()
		{
			// Create proxy calls that override original hooks when enabled
#define OPENGL_CONTEXT_HOOK_PROXY_NAME Proxy
#define OPENGL_CONTEXT_REDIRECTION_NAME OpenGLRedirection

#define OPENGL_CONTEXT_HOOK_PROXY(x) \
			proxy_hooked_function_calls[reinterpret_cast<void*>(OpenGLRedirection::x)] = OPENGL_CONTEXT_HOOK_PROXY_NAME::x; \


#define OPENGL_CONTEXT_HOOK(x) \
			Raysterizer::Hooks::HookFunctionByName(std::string("gl") + #x, OPENGL_CONTEXT_REDIRECTION_NAME::x); \
			OPENGL_CONTEXT_HOOK_PROXY(x); \


			OPENGL_CONTEXT_HOOK(GenVertexArrays);
			OPENGL_CONTEXT_HOOK(GenBuffers);
			OPENGL_CONTEXT_HOOK(DeleteBuffers);

			OPENGL_CONTEXT_HOOK(DeleteVertexArrays);
			OPENGL_CONTEXT_HOOK(BindBuffer);
			OPENGL_CONTEXT_HOOK(BindVertexArray);
			OPENGL_CONTEXT_HOOK(BufferData);
			OPENGL_CONTEXT_HOOK(BufferSubData);
			
			OPENGL_CONTEXT_HOOK(VertexAttribPointer);
			OPENGL_CONTEXT_HOOK(VertexAttribIPointer);
			OPENGL_CONTEXT_HOOK(VertexAttribLPointer);
			OPENGL_CONTEXT_HOOK(EnableVertexAttribArray);
			OPENGL_CONTEXT_HOOK(DisableVertexAttribArray);
			OPENGL_CONTEXT_HOOK(VertexAttribDivisor);
			OPENGL_CONTEXT_HOOK(CreateShader);
			OPENGL_CONTEXT_HOOK(DeleteShader);
			OPENGL_CONTEXT_HOOK(ShaderSource);
			OPENGL_CONTEXT_HOOK(ShaderBinary);
			OPENGL_CONTEXT_HOOK(CompileShader);
			OPENGL_CONTEXT_HOOK(GetShaderiv);
			OPENGL_CONTEXT_HOOK(CreateProgram);
			OPENGL_CONTEXT_HOOK(DeleteProgram);
			OPENGL_CONTEXT_HOOK(AttachShader);
			OPENGL_CONTEXT_HOOK(LinkProgram);
			OPENGL_CONTEXT_HOOK(UseProgram);
			OPENGL_CONTEXT_HOOK(GetProgramiv);
			OPENGL_CONTEXT_HOOK(GetActiveUniform);
			OPENGL_CONTEXT_HOOK(GetUniformLocation);
			OPENGL_CONTEXT_HOOK(GetActiveAttrib);
			OPENGL_CONTEXT_HOOK(GetAttribLocation);
			OPENGL_CONTEXT_HOOK(GetUniformBlockIndex);
			OPENGL_CONTEXT_HOOK(UniformBlockBinding);
			OPENGL_CONTEXT_HOOK(GetActiveUniformBlockiv);
			OPENGL_CONTEXT_HOOK(GetActiveUniformBlockName);

			OPENGL_CONTEXT_HOOK(BindBufferBase);
			OPENGL_CONTEXT_HOOK(BindBufferRange);
			OPENGL_CONTEXT_HOOK(MapBuffer);
			OPENGL_CONTEXT_HOOK(UnmapBuffer);
			OPENGL_CONTEXT_HOOK(MapBufferRange);

			OPENGL_CONTEXT_HOOK(Uniform1f);
			OPENGL_CONTEXT_HOOK(Uniform2f);
			OPENGL_CONTEXT_HOOK(Uniform3f);
			OPENGL_CONTEXT_HOOK(Uniform4f);
			OPENGL_CONTEXT_HOOK(Uniform1i);
			OPENGL_CONTEXT_HOOK(Uniform2i);
			OPENGL_CONTEXT_HOOK(Uniform3i);
			OPENGL_CONTEXT_HOOK(Uniform4i);
			OPENGL_CONTEXT_HOOK(Uniform1ui);
			OPENGL_CONTEXT_HOOK(Uniform2ui);
			OPENGL_CONTEXT_HOOK(Uniform3ui);
			OPENGL_CONTEXT_HOOK(Uniform4ui);
			OPENGL_CONTEXT_HOOK(Uniform1fv);
			OPENGL_CONTEXT_HOOK(Uniform2fv);
			OPENGL_CONTEXT_HOOK(Uniform3fv);
			OPENGL_CONTEXT_HOOK(Uniform4fv);
			OPENGL_CONTEXT_HOOK(Uniform1iv);
			OPENGL_CONTEXT_HOOK(Uniform2iv);
			OPENGL_CONTEXT_HOOK(Uniform3iv);
			OPENGL_CONTEXT_HOOK(Uniform4iv);
			OPENGL_CONTEXT_HOOK(Uniform1uiv);
			OPENGL_CONTEXT_HOOK(Uniform2uiv);
			OPENGL_CONTEXT_HOOK(Uniform3uiv);
			OPENGL_CONTEXT_HOOK(Uniform4uiv);
			OPENGL_CONTEXT_HOOK(UniformMatrix2fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix3fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix4fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix2x3fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix3x2fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix2x4fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix4x2fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix3x4fv);
			OPENGL_CONTEXT_HOOK(UniformMatrix4x3fv);

			OPENGL_CONTEXT_HOOK(GenSamplers);
			OPENGL_CONTEXT_HOOK(DeleteSamplers);
			OPENGL_CONTEXT_HOOK(BindSampler);
			OPENGL_CONTEXT_HOOK(SamplerParameteri);

			OPENGL_CONTEXT_HOOK(GenTextures);
			OPENGL_CONTEXT_HOOK(DeleteTextures);
			OPENGL_CONTEXT_HOOK(ActiveTexture);
			OPENGL_CONTEXT_HOOK(BindTexture);
			OPENGL_CONTEXT_HOOK(TexStorage2D);
			OPENGL_CONTEXT_HOOK(TexStorage3D);
			OPENGL_CONTEXT_HOOK(TexSubImage2D);
			OPENGL_CONTEXT_HOOK(TexSubImage3D);
			OPENGL_CONTEXT_HOOK(TexImage2D);
			OPENGL_CONTEXT_HOOK(TexImage3D);
			OPENGL_CONTEXT_HOOK(GenerateMipmap);
			OPENGL_CONTEXT_HOOK(TexParameteri);
			OPENGL_CONTEXT_HOOK(GenFramebuffers);
			OPENGL_CONTEXT_HOOK(DeleteFramebuffers);
			OPENGL_CONTEXT_HOOK(FramebufferTextureLayer);
			OPENGL_CONTEXT_HOOK(FramebufferTexture);
			OPENGL_CONTEXT_HOOK(BindFramebuffer);

			OPENGL_CONTEXT_HOOK(Clear);
			OPENGL_CONTEXT_HOOK(DrawArrays);
			OPENGL_CONTEXT_HOOK(DrawElements);
			OPENGL_CONTEXT_HOOK(DrawElementsBaseVertex);
			OPENGL_CONTEXT_HOOK(DrawElementsInstanced);

			OPENGL_CONTEXT_HOOK(Viewport);
			OPENGL_CONTEXT_HOOK(ClearColor);
			OPENGL_CONTEXT_HOOK(ClearDepth);
			OPENGL_CONTEXT_HOOK(DepthFunc);
			OPENGL_CONTEXT_HOOK(DepthRange);

			OPENGL_CONTEXT_HOOK(BlendColor);
			OPENGL_CONTEXT_HOOK(BlendEquation);
			OPENGL_CONTEXT_HOOK(BlendEquationSeparate);
			OPENGL_CONTEXT_HOOK(BlendFuncSeparate);
			OPENGL_CONTEXT_HOOK(ColorMask);
			OPENGL_CONTEXT_HOOK(LogicOp);

			OPENGL_CONTEXT_HOOK(Disable);
			OPENGL_CONTEXT_HOOK(Enable);
			OPENGL_CONTEXT_HOOK(DepthMask);
			OPENGL_CONTEXT_HOOK(GetString);
			OPENGL_CONTEXT_HOOK(GetStringi);
			OPENGL_CONTEXT_HOOK(GetIntegerv);

			Raysterizer::Hooks::HookFunctionByName("wglSwapBuffers", OPENGL_CONTEXT_REDIRECTION_NAME::hook_wglSwapBuffers);
		}
	}
}