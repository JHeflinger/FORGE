#include "Framebuffer.h"
#include "Core/Safety.h"
#include <glad/glad.h>

static const uint32_t s_MaxFrameBufferSize = 8196;

static GLenum TextureTarget(bool multisampled) {
	return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count) {
	glCreateTextures(TextureTarget(multisampled), count, outID);
}

static void BindTexture(bool multisampled, uint32_t id) {
	glBindTexture(TextureTarget(multisampled), id);
}

static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index) {
	bool multisampled = samples > 1;
	if (multisampled) {
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
}

static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height) {
	bool multisampled = samples > 1;
	if (multisampled) {
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
	} else {
		glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
}

static bool IsDepthFormat(FramebufferTextureFormat format) {
	switch (format) {
	case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
	default: return false;
	}
}

static GLenum FloraFBTextureFormatToGL(FramebufferTextureFormat format) {
	switch (format) {
	case FramebufferTextureFormat::RGBA8: return GL_RGBA8;
	case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
	default: ASSERT(false, "Invalid texture format!");
	}
	return 0;
}

Framebuffer::Framebuffer(const FramebufferSpecification& spec)
	: m_Specification(spec) {
	for (auto format : m_Specification.Attachments.Attachments) {
		if (!IsDepthFormat(format.TextureFormat))
			m_ColorAttachmentSpecifications.emplace_back(format);
		else
			m_DepthAttachmentSpecification = format;
	}
	Invalidate();
}

Framebuffer::~Framebuffer() {
	glDeleteFramebuffers(1, &m_RendererID);
	glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
	glDeleteTextures(1, &m_DepthAttachment);
}

void Framebuffer::Invalidate() {
	if (m_RendererID) {
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
		m_ColorAttachments.clear();
		m_DepthAttachment = 0;
	}

	glCreateFramebuffers(1, &m_RendererID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

	// Attachments
	bool multisample = m_Specification.Samples > 1;
	if (m_ColorAttachmentSpecifications.size()) {
		m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
		CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());
		for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
			BindTexture(multisample, m_ColorAttachments[i]);
			switch (m_ColorAttachmentSpecifications[i].TextureFormat) {
			case FramebufferTextureFormat::RGBA8:
				AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i);
				break;
			case FramebufferTextureFormat::RED_INTEGER:
				AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i);
				break;
			default: break;
			}
		}
	}

	if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None) {
		CreateTextures(multisample, &m_DepthAttachment, 1);
		BindTexture(multisample, m_DepthAttachment);
		switch (m_DepthAttachmentSpecification.TextureFormat) {
		case FramebufferTextureFormat::DEPTH24STENCIL8:
			AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height);
			break;
		default: break;
		}
	}

	if (m_ColorAttachments.size() > 1) {
		ASSERT(m_ColorAttachments.size() <= 4, "There currently is not support for more than 4 color attachments!");
		GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(m_ColorAttachments.size(), buffers);
	} else if (m_ColorAttachments.empty()) {
		// Only depth-pass
		glDrawBuffer(GL_NONE);
	}

	ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
	glViewport(0, 0, m_Specification.Width, m_Specification.Height);
}

void Framebuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0 || width > s_MaxFrameBufferSize || height > s_MaxFrameBufferSize) {
		WARN("Attempt to resize framebuffer to an unreasonable size detected");
		return;
	}
	m_Specification.Width = width;
	m_Specification.Height = height;
	Invalidate();
}

int Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) {
	ASSERT(attachmentIndex < m_ColorAttachments.size(), "Cannot read pixel from invalid attachment!");
	glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
	int pixelData;
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
	return pixelData;
}

void Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value) {
	ASSERT(attachmentIndex < m_ColorAttachments.size(), "Cannot clear an invalid attachment!");
	auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
	glClearTexImage(m_ColorAttachments[attachmentIndex], 0, 
		FloraFBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
}
