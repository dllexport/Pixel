#include <Engine/Camera.h>

Camera::Camera(IntrusivePtr<RHIRuntime> rhiRuntime) : rhiRuntime(rhiRuntime)
{
    this->Allocate();
}

void Camera::Allocate()
{
    this->uniformBuffer = rhiRuntime->CreateMutableBuffer(Buffer::BUFFER_USAGE_UNIFORM_BUFFER_BIT, MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT | MemoryProperty::MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(UBO));
}

bool Camera::EventCallback(UpdateInput inputs)
{
    auto event = inputs.event;
    auto deltaTime = inputs.deltaTime;

    if (event.type == Event::RESIZE)
    {
        this->setPerspective(60.0f, (float)event.resizeEvent.width / (float)event.resizeEvent.width, 0.1f, 256.0f);
    }

    if (event.type == Event::KEY_UP)
    {
        if (event.keyEvent.keyCode == EVENT_KEY_W)
            this->keys.up = false;
        if (event.keyEvent.keyCode == EVENT_KEY_A)
            this->keys.left = false;
        if (event.keyEvent.keyCode == EVENT_KEY_S)
            this->keys.down = false;
        if (event.keyEvent.keyCode == EVENT_KEY_D)
            this->keys.right = false;
    }

    auto anyKeyDown = event.type & Event::KEY_DOWN || event.type & Event::KEY_REPEAT;
    if (anyKeyDown)
    {
        if (event.keyEvent.keyCode == EVENT_KEY_W)
            this->keys.up = true;
        if (event.keyEvent.keyCode == EVENT_KEY_A)
            this->keys.left = true;
        if (event.keyEvent.keyCode == EVENT_KEY_S)
            this->keys.down = true;
        if (event.keyEvent.keyCode == EVENT_KEY_D)
            this->keys.right = true;
    }

    if (event.type == Event::MOUSE_MOVE)
    {
        if (inputs.ioState.mouse[0])
        {
            if (mousePos.x == 0 && mousePos.y == 0)
            {
                mousePos = {event.keyEvent.mouseX, event.keyEvent.mouseY};
            }
            int32_t dx = (int32_t)mousePos.x - event.keyEvent.mouseX;
            int32_t dy = (int32_t)mousePos.y - event.keyEvent.mouseY;

            this->rotate(glm::vec3(dy * this->rotationSpeed * deltaTime / 10.f, -dx * this->rotationSpeed * deltaTime / 10.f, 0.0f));

            mousePos = {event.keyEvent.mouseX, event.keyEvent.mouseY};
        }
    }

    if (event.type == Event::MOUSE_UP)
    {
        mousePos = {};
    }

    if (event.type == Event::MOUSE_SCROLL)
    {
        // TODO: handle scroll
    }

    this->update(deltaTime / 100.f);

    auto frameUniformBuffer = static_cast<MutableBuffer*>(uniformBuffer.get());

    memcpy(frameUniformBuffer->GetBuffer(inputs.currentImageIndex)->Map(), &ubo, sizeof(ubo));

    return false;
}