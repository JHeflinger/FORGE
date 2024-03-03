#include "Window.h"
#include "../Events/KeyEvent.h"
#include "../Events/MouseEvent.h"
#include "../Events/ApplicationEvent.h"
#include "Log.h"
#include "Safety.h"

static bool s_GLFWInitialized = false;
static void GLFWErrorCallback(int error, const char* desc) {
	FATAL("GLFW Error ({0}): {1}", error, desc);
}

Window::Window(const WindowProperties& properties) {
	m_Data.Name = properties.Name;
	m_Data.Width = properties.Width;
	m_Data.Height = properties.Height;
	m_Data.Fullscreen = properties.Fullscreen;
	INFO("Creating window {0} {1}, {2}", properties.Name, properties.Width, properties.Height);
	if (!s_GLFWInitialized) {
		int success = glfwInit();
		ASSERT(success, "Failed to initialize GLFW");
		glfwSetErrorCallback(GLFWErrorCallback);
		s_GLFWInitialized = true;
	}

	m_Window = glfwCreateWindow((int)properties.Width, (int)properties.Height, properties.Name.c_str(), nullptr, nullptr);
	if (properties.Fullscreen) glfwMaximizeWindow(m_Window);
	glfwMakeContextCurrent(m_Window);
	int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	ASSERT(status, "Failed to initialize glad");
	INFO("OpenGL Info:\n\tVendor:{0}\n\tRenderer: {1}\n\tVersion: {2}", (char*)glGetString(GL_VENDOR), (char*)glGetString(GL_RENDERER), (char*)glGetString(GL_VERSION));
	int versionMajor, versionMinor;
	glGetIntegerv(GL_MAJOR_VERSION, &versionMajor);
	glGetIntegerv(GL_MINOR_VERSION, &versionMinor);
	ASSERT(versionMajor > 4 || (versionMajor == 4 && versionMinor >= 5), "Forge requires at least OpenGL version 4.5!");

	glfwSetWindowUserPointer(m_Window, &m_Data);
	SetVSync(true);

	glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;
		WindowResizeEvent event(width, height);
		data.EventCallback(event);
	});

	glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {	
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		WindowCloseEvent event;
		data.EventCallback(event);
	});

	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {	
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		switch(action) {
			case GLFW_PRESS: {
				KeyPressedEvent event(static_cast<KeyCode>(key), 0);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE: {
				KeyReleasedEvent event(static_cast<KeyCode>(key));
				data.EventCallback(event);
				break;
			}
			case GLFW_REPEAT: {
				KeyPressedEvent event(static_cast<KeyCode>(key), 1);
				data.EventCallback(event);
				break;
			}
			default: break;
		}
	});

	glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {	
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		KeyTypedEvent event(static_cast<KeyCode>(keycode));
		data.EventCallback(event);
	});

	glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		switch(action) {
			case GLFW_PRESS: {
				MouseButtonPressedEvent event(button);
				data.EventCallback(event);
				break;
			}
			case GLFW_RELEASE: {
				MouseButtonReleasedEvent event(button);
				data.EventCallback(event);
				break;
			}
		}
	});

	glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		MouseScrolledEvent event((float)xOffset, (float)yOffset);
		data.EventCallback(event);
	});
	
	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		MouseMovedEvent event((float)xPos, (float)yPos);
		data.EventCallback(event);
	});
}

Window::~Window() {
	glfwDestroyWindow(m_Window);
}
	
void Window::Update() {
	glfwPollEvents();
	glfwSwapBuffers(m_Window);
}

void Window::SetVSync(bool enabled) {
	glfwSwapInterval(enabled? 1 : 0);
	m_Data.VSync = enabled;
}
