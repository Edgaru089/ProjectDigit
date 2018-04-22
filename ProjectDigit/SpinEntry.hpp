#pragma once

#include <SFGUI/Widgets.hpp>
#include <SFGUI/RenderQueue.hpp>
#include <SFML/Graphics.hpp>
#include <memory>

#include "LogSystem.hpp"

using namespace sfg;
using namespace std;

class SpinEntry:public SpinButton {

public:

	bool IsStepperPressed() const {
		return m_pressed;
	}

private:

	void HandleMouseMoveEvent(int x, int y) override {

	}

	void HandleMouseButtonEvent(sf::Mouse::Button button, bool press, int x, int y) override {
		float border_width(Context::Get().GetEngine().GetProperty<float>("BorderWidth", shared_from_this()));
		float stepper_aspect_ratio(Context::Get().GetEngine().GetProperty<float>("StepperAspectRatio", shared_from_this()));

		if (button != sf::Mouse::Left) {
			return;
		}

		auto stepper_height = (GetAllocation().height / 2.f) - border_width;
		auto stepper_width = (GetAllocation().height / 2.f) * stepper_aspect_ratio;

		if (press) {
			// Top stepper.
			sf::FloatRect rect;
			rect.left = GetAllocation().left + GetAllocation().width - border_width - stepper_width;
			rect.top = GetAllocation().top + border_width;
			rect.width = stepper_width;
			rect.height = stepper_height;

			if (rect.contains(static_cast<float>(x), static_cast<float>(y))) {
				GrabFocus(Widget::Ptr());

				m_adjustment->Increment();

				m_elapsed_time = 0.f;
				m_increase_pressed = true;
				m_repeat_wait = true;

				Invalidate();
				return;
			}

			// Bottom stepper.
			rect.top = GetAllocation().top + border_width + stepper_height;

			if (rect.contains(static_cast<float>(x), static_cast<float>(y))) {
				GrabFocus(Widget::Ptr());

				m_adjustment->Decrement();

				m_elapsed_time = 0.f;
				m_decrease_pressed = true;
				m_repeat_wait = true;

				Invalidate();
				return;
			}
		}
		else {
			if (m_decrease_pressed || m_increase_pressed) {
				Invalidate();
			}

			m_decrease_pressed = false;
			m_increase_pressed = false;
		}

		Entry::HandleMouseButtonEvent(button, press, x, y);
	}

	bool m_pressed;

	std::shared_ptr<Adjustment> m_adjustment;

	float m_elapsed_time;

	unsigned int m_adjustment_signal_serial;
	unsigned int m_digits;

	bool m_decrease_pressed;
	bool m_increase_pressed;

	bool m_repeat_wait;

protected:

	unique_ptr<RenderQueue> InvalidateImpl() const override {
		mlogd << "A SpinEntry was invalidated!" << dlog;

		auto border_color = Context::Get().GetEngine().GetProperty<sf::Color>("BorderColor", shared_from_this());
		auto background_color = Context::Get().GetEngine().GetProperty<sf::Color>("BackgroundColor", shared_from_this());
		auto text_color = Context::Get().GetEngine().GetProperty<sf::Color>("Color", shared_from_this());
		auto cursor_color = Context::Get().GetEngine().GetProperty<sf::Color>("Color", shared_from_this());
		auto text_padding = Context::Get().GetEngine().GetProperty<float>("Padding", shared_from_this());
		auto cursor_thickness = Context::Get().GetEngine().GetProperty<float>("Thickness", shared_from_this());
		auto border_width = Context::Get().GetEngine().GetProperty<float>("BorderWidth", shared_from_this());
		auto border_color_shift = Context::Get().GetEngine().GetProperty<int>("BorderColorShift", shared_from_this());
		const auto& font_name = Context::Get().GetEngine().GetProperty<std::string>("FontName", shared_from_this());
		const auto& font = Context::Get().GetEngine().GetResourceManager().GetFont(font_name);
		auto font_size = Context::Get().GetEngine().GetProperty<unsigned int>("FontSize", shared_from_this());
		auto stepper_aspect_ratio = Context::Get().GetEngine().GetProperty<float>("StepperAspectRatio", shared_from_this());
		auto stepper_color = Context::Get().GetEngine().GetProperty<sf::Color>("StepperBackgroundColor", shared_from_this());
		auto stepper_border_color = Context::Get().GetEngine().GetProperty<sf::Color>("BorderColor", shared_from_this());
		auto stepper_arrow_color = Context::Get().GetEngine().GetProperty<sf::Color>("StepperArrowColor", shared_from_this());

		std::unique_ptr<RenderQueue> queue(new RenderQueue);

		// Pane.
		queue->Add(
			Renderer::Get().CreatePane(
				sf::Vector2f(0.f, 0.f),
				sf::Vector2f(this->GetAllocation().width, this->GetAllocation().height),
				border_width,
				background_color,
				border_color,
				-border_color_shift
			)
		);

		auto button_width = (this->GetAllocation().height / 2.f) * stepper_aspect_ratio;

		// Stepper.
		queue->Add(
			Renderer::Get().CreatePane(
				sf::Vector2f(this->GetAllocation().width - button_width - border_width, border_width),
				sf::Vector2f(button_width, this->GetAllocation().height - border_width),
				border_width,
				stepper_color,
				stepper_border_color,
				this->IsIncreaseStepperPressed() ? -border_color_shift : border_color_shift
			)
		);

		// Up Stepper Triangle.
		queue->Add(
			Renderer::Get().CreateTriangle(
				sf::Vector2f(this->GetAllocation().width - button_width / 2.f - border_width, (this->IsStepperPressed() ? 1.f : 0.f) + border_width + this->GetAllocation().height / 6.f),
				sf::Vector2f(this->GetAllocation().width - button_width / 4.f * 3.f - border_width, (this->IsStepperPressed() ? 1.f : 0.f) + border_width + this->GetAllocation().height / 3.f),
				sf::Vector2f(this->GetAllocation().width - button_width / 4.f - border_width, (this->IsStepperPressed() ? 1.f : 0.f) + border_width + this->GetAllocation().height / 3.f),
				stepper_arrow_color
			)
		);

		// Down Stepper Triangle.
		queue->Add(
			Renderer::Get().CreateTriangle(
				sf::Vector2f(this->GetAllocation().width - button_width / 2.f - border_width, (this->IsStepperPressed() ? 1.f : 0.f) + this->GetAllocation().height - border_width - this->GetAllocation().height / 6.f),
				sf::Vector2f(this->GetAllocation().width - button_width / 4.f - border_width, (this->IsStepperPressed() ? 1.f : 0.f) + this->GetAllocation().height - border_width - this->GetAllocation().height / 3.f),
				sf::Vector2f(this->GetAllocation().width - button_width / 4.f * 3.f - border_width, (this->IsStepperPressed() ? 1.f : 0.f) + this->GetAllocation().height - border_width - this->GetAllocation().height / 3.f),
				stepper_arrow_color
			)
		);

		auto line_height = Context::Get().GetEngine().GetFontLineHeight(*font, font_size);
		sf::Text vis_label(this->GetVisibleText(), *font, font_size);
		vis_label.setFillColor(text_color);
		vis_label.setPosition(text_padding, this->GetAllocation().height / 2.f - line_height / 2.f);

		queue->Add(Renderer::Get().CreateText(vis_label));

		// Draw cursor if this is active and cursor is visible.
		if (this->HasFocus() && this->IsCursorVisible()) {
			sf::String cursor_string(this->GetVisibleText());
			if (this->GetCursorPosition() - this->GetVisibleOffset() < static_cast<int>(cursor_string.getSize())) {
				cursor_string.erase(static_cast<std::size_t>(this->GetCursorPosition() - this->GetVisibleOffset()), cursor_string.getSize());
			}

			// Get metrics.
			sf::Vector2f metrics(Context::Get().GetEngine().GetTextStringMetrics(cursor_string, *font, font_size));

			queue->Add(
				Renderer::Get().CreateRect(
					sf::FloatRect(
						metrics.x + text_padding,
						this->GetAllocation().height / 2.f - line_height / 2.f,
						cursor_thickness,
						line_height
					),
					cursor_color
				)
			);
		}

		return queue;
	}

};
