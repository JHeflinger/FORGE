#pragma once
#include "../Core/Safety.h"

enum class FramebufferTextureFormat {
	None = 0,

	//Color
	RGBA8,
	RED_INTEGER,

	// Depth/stencil
	DEPTH24STENCIL8,

	// Defaults
	Depth = DEPTH24STENCIL8
};

struct FramebufferTextureSpecification {
	FramebufferTextureSpecification() = default;
	FramebufferTextureSpecification(FramebufferTextureFormat format)
		: TextureFormat(format) {}
	FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
};

struct FramebufferAttatchmentSpecification {
	FramebufferAttatchmentSpecification() = default;
	FramebufferAttatchmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
		: Attachments(attachments) {}
	std::vector<FramebufferTextureSpecification> Attachments;
};

struct FramebufferSpecification {
	uint32_t Width, Height = 0;
	FramebufferAttatchmentSpecification Attachments;
	uint32_t Samples = 1;
	bool SwapChainTarget = false;
};

class Framebuffer {
public:
	Framebuffer(const FramebufferSpecification& spec);
	~Framebuffer();
	void Invalidate();
	void Bind();
	void Unbind();
	void Resize(uint32_t width, uint32_t height);
	void ClearAttachment(uint32_t attachmentIndex, int value);
	int ReadPixel(uint32_t attachmentIndex, int x, int y);
	uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const { ASSERT(m_ColorAttachments.size() >= index, "Invalid color attachment index!"); return m_ColorAttachments[index]; }
	const FramebufferSpecification& GetSpecification() const { return m_Specification; }
private:
	uint32_t m_RendererID = 0;
	uint32_t m_DepthAttachment = 0;
	FramebufferSpecification m_Specification;
	FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;
	std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
	std::vector<uint32_t> m_ColorAttachments;
};
