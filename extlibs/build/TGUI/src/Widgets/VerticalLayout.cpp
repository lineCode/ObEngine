/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus' Graphical User Interface
// Copyright (C) 2012-2017 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <TGUI/Widgets/VerticalLayout.hpp>
#include <numeric>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VerticalLayout::VerticalLayout()
    {
        m_callback.widgetType = "VerticalLayout";
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VerticalLayout::Ptr VerticalLayout::create()
    {
        return std::make_shared<VerticalLayout>();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    VerticalLayout::Ptr VerticalLayout::copy(ConstPtr layout)
    {
        if (layout)
            return std::static_pointer_cast<VerticalLayout>(layout->clone());
        return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void VerticalLayout::updateWidgetPositions()
    {
        float sumRatio = 0;
        for (size_t i = 0; i < m_widgetsRatio.size(); ++i)
        {
            if (m_widgetsFixedSizes[i] == 0)
                sumRatio += m_widgetsRatio[i];
        }

        float currentRatio = 0;
        float currentOffset = 0;
        const float sumFixedSize = accumulate(m_widgetsFixedSizes.begin(), m_widgetsFixedSizes.end(), 0.f);
        for (size_t i = 0; i < m_widgets.size(); ++i)
        {
            m_widgets[i]->setPosition(0.f, (getSize().y - sumFixedSize) * currentRatio + currentOffset);
            if (m_widgetsFixedSizes[i])
            {
                m_widgets[i]->setSize(getSize().x, m_widgetsFixedSizes[i]);
                currentOffset += m_widgetsFixedSizes[i];
            }
            else
            {
                m_widgets[i]->setSize(getSize().x, (getSize().y - sumFixedSize) * m_widgetsRatio[i] / sumRatio);
                currentRatio += m_widgetsRatio[i] / sumRatio;
            }

            // Correct the size for widgets that have borders around it or a text next to them
            if (m_widgets[i]->getFullSize() != m_widgets[i]->getSize())
            {
                sf::Vector2f newSize = m_widgets[i]->getSize() - (m_widgets[i]->getFullSize() - m_widgets[i]->getSize());
                if (newSize.x > 0 && newSize.y > 0)
                {
                    m_widgets[i]->setSize(newSize);
                    m_widgets[i]->setPosition(m_widgets[i]->getPosition() - m_widgets[i]->getWidgetOffset());
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
