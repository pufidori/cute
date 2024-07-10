#include "includes.h"

// Constructor is already defined inline in the header, so it's not needed here

void Checkbox::draw() {
	Rect area{ m_parent->GetElementsRect() };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// get gui color.
	Color color = g_gui.m_color;
	color.a() = m_parent->m_alpha;

	// render black outline on checkbox.
	render::rect(p.x, p.y, CHECKBOX_SIZE, CHECKBOX_SIZE, { 10, 10, 10, m_parent->m_alpha });

	// render checkbox title.
	if (m_use_label)
		render::menu_shade.string(p.x + LABEL_OFFSET, p.y - 3, { 205, 205, 205, m_parent->m_alpha }, m_label);

	// render border.
	render::rect(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 0, 0, 0, m_parent->m_alpha });

	// Smooth animation for the checked state.
	if (m_checked) {
		m_animation_progress = std::min(1.0f, m_animation_progress + g_csgo.m_globals->m_frametime * 10);
	}
	else {
		m_animation_progress = std::max(0.0f, m_animation_progress - g_csgo.m_globals->m_frametime * 10);
	}

	// Interpolated color based on animation progress.
	Color interpolated_color = color;
	interpolated_color.a() = static_cast<int>(m_parent->m_alpha * m_animation_progress);

	// render checked with smooth transition.
	render::rect_filled(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, interpolated_color);
	render::rect_filled_fade(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 50, 50, 35, static_cast<int>(m_parent->m_alpha * m_animation_progress) }, 0, 150);

	// render unchecked state with gradient.
	if (m_animation_progress < 1.0f) {
		render::gradient(p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 75, 75, 75, static_cast<int>(m_parent->m_alpha * (1.0f - m_animation_progress)) }, { 50, 50, 50, static_cast<int>(m_parent->m_alpha * (1.0f - m_animation_progress)) });
	}
}

void Checkbox::think() {
	// set the click area to the length of the string, so we can also press the string to toggle.
	if (m_use_label)
		m_w = LABEL_OFFSET + render::menu_shade.size(m_label).m_width;
}

void Checkbox::click() {
	// toggle.
	m_checked = !m_checked;

	if (m_callback)
		m_callback();
}
