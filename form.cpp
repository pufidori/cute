#include "includes.h"

void Form::draw( ) {
	// opacity should reach 1 in 150 milliseconds.
	constexpr float frequency = 1.f / 0.15f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// if open		-> increment
	// if closed	-> decrement
	m_open ? m_opacity += step : m_opacity -= step;

	// clamp the opacity.
	math::clamp( m_opacity, 0.f, 1.f );

	m_alpha = 0xff * m_opacity;
	if( !m_alpha )
		return;

	// get gui color.
	Color color = g_gui.m_color;
	color.a( ) = m_alpha;
        // Background
    render::rect_filled(m_x, m_y, m_width, m_height, { 23, 23, 23, m_alpha });
	for (int io = 0; io < m_width / 2 - 5; io++) {
		//render::rect(m_x + 5 + (io * 2), m_y + 5, 1, m_height - 11, { 30, 40, 45, m_alpha }); // 20,20,20 - shit menu pattern
	}

	// lbgt line
	//render::gradient1337(m_x + 7, m_y + 7, m_width / 2 - 7, 1, { 99, 160, 200, m_alpha }, { 179, 102, 181, m_alpha });
	//render::gradient1337(m_x + (m_width / 2), m_y + 7, m_width / 2 - 7, 1, { 179, 102, 181, m_alpha }, { 230, 217, 100, m_alpha });

	//render::gradient1337(m_x + 7, m_y + 7 + 1, m_width / 2 - 7, 1, { 49, 79, 99, m_alpha }, { 89, 50, 90, m_alpha });
	//render::gradient1337(m_x + (m_width / 2), m_y + 7 + 1, m_width / 2 - 7, 1, { 89, 50, 90, m_alpha }, { 114, 108, 49, m_alpha });

    //cute line :3
    render::gradient1337(m_x + 7, m_y + 7, m_width / 2 - 7, 1, { 189, 207, 253, m_alpha }, { 253, 189, 202, m_alpha });
    render::gradient1337(m_x + (m_width / 2), m_y + 7, m_width / 2 - 7, 1, { 253, 189, 202, m_alpha }, { 189, 207, 253, m_alpha });
    render::gradient1337(m_x + 7, m_y + 7 + 1, m_width / 2 - 7, 1, { 189, 207, 253, m_alpha }, { 253, 189, 202, m_alpha });
    render::gradient1337(m_x + (m_width / 2), m_y + 7 + 1, m_width / 2 - 7, 1, { 253, 189, 202, m_alpha }, { 189, 207, 253, m_alpha });


	// border.
	render::rect(m_x, m_y, m_width, m_height, { 5, 5, 5, m_alpha });
	render::rect(m_x + 1, m_y + 1, m_width - 2, m_height - 2, { 60, 60, 60, m_alpha });
	render::rect(m_x + 2, m_y + 2, m_width - 4, m_height - 4, { 40, 40, 40, m_alpha });
	render::rect(m_x + 3, m_y + 3, m_width - 6, m_height - 6, { 40, 40, 40, m_alpha });
	render::rect(m_x + 4, m_y + 4, m_width - 8, m_height - 8, { 40, 40, 40, m_alpha });
	render::rect(m_x + 5, m_y + 5, m_width - 10, m_height - 10, { 60, 60, 60, m_alpha });


	// draw tabs if we have any.
	if( !m_tabs.empty( ) ) {
		// tabs background and border.
		Rect tabs_area = GetTabsRect( );

        // Calculate button dimensions
        int buttonWidth = 80; // Adjust button width as needed
        int buttonHeight = 16; // Adjust button height as needed
        int buttonSpacing = 4; // Adjust spacing between buttons as needed

        // Iterate through tabs and render them as buttons
        int yOffset = 0;
        for (size_t i = 0; i < m_tabs.size(); ++i) {
            const auto& tab = m_tabs[i];

            // Calculate button position
            int buttonX = tabs_area.x + 10;
            int buttonY = tabs_area.y + 5 + yOffset;

            // Determine if mouse is over the button
            bool mouseOver = g_input.IsCursorInBounds(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight);

            // Handle tab switching on click
            if (mouseOver && g_input.GetKeyPress(VK_LBUTTON)) {
                m_active_tab = tab;
                break; // Exit loop early since tab is already switched
            }

            // Render button background
            render::rect_filled(buttonX, buttonY, buttonWidth, buttonHeight, tab == m_active_tab ? Color{ color.r(), color.g(), color.b(), 50 } : Color{ 27, 27, 27, 50 });

            // Render button outline
            //render::rect(buttonX, buttonY, buttonWidth, buttonHeight, { 5, 5, 5, m_alpha });

            // Render button text
            render::menu_shade.string(buttonX + 5, buttonY + 2, tab == m_active_tab ? colors::white : Color{ 152, 152, 152, m_alpha }, tab->m_title);

            // Update yOffset for the next button
            yOffset += buttonHeight + buttonSpacing;
        }

        // Draw elements for the active tab if it has any
        if (!m_active_tab->m_elements.empty()) {
            Rect el = GetElementsRect();
           // render::rect(el.x, el.y, el.w, el.h, { 0, 0, 0, m_alpha });
           // render::rect(el.x + 1, el.y + 1, el.w - 2, el.h - 2, { 48, 48, 48, m_alpha });

            for (const auto& e : m_active_tab->m_elements) {
                if (!e || (m_active_element && e == m_active_element))
                    continue;
                if (!e->m_show)
                    continue;
                if (!(e->m_flags & ElementFlags::DRAW))
                    continue;
                e->draw();
            }

            if (m_active_element && m_active_element->m_show)
                m_active_element->draw();
        }
    }
}