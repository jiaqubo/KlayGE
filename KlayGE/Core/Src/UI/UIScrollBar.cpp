// UIScrollBar.cpp
// KlayGE 图形用户界面滚动条类 实现文件
// Ver 3.6.0
// 版权所有(C) 龚敏敏, 2007
// Homepage: http://klayge.sourceforge.net
//
// 3.6.0
// 初次建立 (2007.6.28)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>

#include <boost/bind.hpp>

#include <KlayGE/UI.hpp>

namespace KlayGE
{
	// Minimum scroll bar thumb size
	int const SCROLLBAR_MINTHUMBSIZE = 8;

	// Delay and repeat period when clicking on the scroll bar arrows
	float const SCROLLBAR_ARROWCLICK_DELAY = 0.33f;
	float const SCROLLBAR_ARROWCLICK_REPEAT = 0.05f;

	Timer UIScrollBar::timer_;


	UIScrollBar::UIScrollBar(UIDialogPtr dialog)
					: UIControl(UIScrollBar::Type, dialog),
						show_thumb_(true), drag_(false),
						position_(0), page_size_(1),
						start_(0), end_(1),
						arrow_(CLEAR), arrow_ts_(0)
	{
		up_button_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		down_button_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		track_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		thumb_rc_ = Rect_T<int32_t>(0, 0, 0, 0);

		this->InitDefaultElements();
	}

	UIScrollBar::UIScrollBar(UIDialogPtr dialog, int ID, int x, int y, int width, int height, int nTrackStart, int nTrackEnd, int nTrackPos, int nPageSize)
					: UIControl(UIScrollBar::Type, dialog),
						show_thumb_(true), drag_(false),
						position_(nTrackPos), page_size_(nPageSize),
						start_(nTrackStart), end_(nTrackEnd),
						arrow_(CLEAR), arrow_ts_(0)
	{
		up_button_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		down_button_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		track_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		thumb_rc_ = Rect_T<int32_t>(0, 0, 0, 0);

		this->InitDefaultElements();

		// Set the ID and position
		this->SetID(ID);
		this->SetLocation(x, y);
		this->SetSize(width, height);
	}

	UIScrollBar::UIScrollBar(uint32_t type, UIDialogPtr dialog)
					: UIControl(type, dialog),
						show_thumb_(true), drag_(false),
						position_(0), page_size_(1),
						start_(0), end_(1),
						arrow_(CLEAR), arrow_ts_(0)
	{
		up_button_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		down_button_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		track_rc_ = Rect_T<int32_t>(0, 0, 0, 0);
		thumb_rc_ = Rect_T<int32_t>(0, 0, 0, 0);

		this->InitDefaultElements();
	}

	void UIScrollBar::InitDefaultElements()
	{
		UIElement Element;

		// Track
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 0));
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 1);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Up Arrow
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 1));
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 1);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Down Arrow
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 2));
			Element.TextureColor().States[UICS_Disabled] = Color(200.0f / 255, 200.0f / 255, 200.0f / 255, 1);

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		// Button
		{
			Element.SetTexture(0, UIManager::Instance().ElementTextureRect(UICT_ScrollBar, 3));

			elements_.push_back(UIElementPtr(new UIElement(Element)));
		}

		mouse_over_event_.connect(boost::bind(&UIScrollBar::MouseOverHandler, this, _1, _2));
		mouse_down_event_.connect(boost::bind(&UIScrollBar::MouseDownHandler, this, _1, _2));
		mouse_up_event_.connect(boost::bind(&UIScrollBar::MouseUpHandler, this, _1, _2));
	}

	UIScrollBar::~UIScrollBar()
	{
	}

	void UIScrollBar::UpdateRects()
	{
		UIControl::UpdateRects();

		// Make the buttons square

		up_button_rc_ = Rect_T<int32_t>(bounding_box_.left(), bounding_box_.top(),
			bounding_box_.right(), bounding_box_.top() + bounding_box_.Width());
		down_button_rc_ = Rect_T<int32_t>(bounding_box_.left(), bounding_box_.bottom() - bounding_box_.Width(),
			bounding_box_.right(), bounding_box_.bottom());
		track_rc_ = Rect_T<int32_t>(up_button_rc_.left(), up_button_rc_.bottom(),
			down_button_rc_.right(), down_button_rc_.top());
		thumb_rc_.left() = up_button_rc_.left();
		thumb_rc_.right() = up_button_rc_.right();

		this->UpdateThumbRect();

		bounding_box_ = up_button_rc_ | down_button_rc_ | track_rc_ | thumb_rc_;
	}

	// Compute the dimension of the scroll thumb
	void UIScrollBar::UpdateThumbRect()
	{
		if (end_ - start_ > page_size_)
		{
			int nThumbHeight = std::max(static_cast<int>(track_rc_.Height() * page_size_ / (end_ - start_)), SCROLLBAR_MINTHUMBSIZE);
			size_t nMaxPosition = end_ - start_ - page_size_;
			thumb_rc_.top() = track_rc_.top() + static_cast<int>((position_ - start_) * (track_rc_.Height() - nThumbHeight) / nMaxPosition);
			thumb_rc_.bottom() = thumb_rc_.top() + nThumbHeight;
			show_thumb_ = true;
		}
		else
		{
			// No content to scroll
			thumb_rc_.bottom() = thumb_rc_.top();
			show_thumb_ = false;
		}
	}

	// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
	// value scrolls up.
	void UIScrollBar::Scroll(int nDelta)
	{
		// Perform scroll
		int new_pos = position_ + nDelta;
		position_ = MathLib::clamp(new_pos, 0, static_cast<int>(end_) - 1);

		// Cap position
		this->Cap();

		// Update thumb position
		this->UpdateThumbRect();
	}

	void UIScrollBar::ShowItem(size_t nIndex)
	{
		// Cap the index

		if (nIndex < 0)
		{
			nIndex = 0;
		}

		if (nIndex >= end_)
		{
			nIndex = end_ - 1;
		}

		// Adjust position

		if (position_ > nIndex)
		{
			position_ = nIndex;
		}
		else
		{
			if (position_ + page_size_ <= nIndex)
			{
				position_ = nIndex - page_size_ + 1;
			}
		}

		this->UpdateThumbRect();
	}

	void UIScrollBar::MouseOverHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		last_mouse_ = arg.location;
		is_mouse_over_ = true;

		if (drag_)
		{
			thumb_rc_.bottom() += arg.location.y() - thumb_offset_y_ - thumb_rc_.top();
			thumb_rc_.top() = arg.location.y() - thumb_offset_y_;
			if (thumb_rc_.top() < track_rc_.top())
			{
				thumb_rc_ += Vector_T<int32_t, 2>(0, track_rc_.top() - thumb_rc_.top());
			}
			else
			{
				if (thumb_rc_.bottom() > track_rc_.bottom())
				{
					thumb_rc_ += Vector_T<int32_t, 2>(0, track_rc_.bottom() - thumb_rc_.bottom());
				}
			}

			// Compute first item index based on thumb position

			size_t nMaxFirstItem = end_ - start_ - page_size_;  // Largest possible index for first item
			size_t nMaxThumb = track_rc_.Height() - thumb_rc_.Height();  // Largest possible thumb position from the top

			position_ = start_ +
				(thumb_rc_.top() - track_rc_.top() +
				nMaxThumb / (nMaxFirstItem * 2)) * // Shift by half a row to avoid last row covered by only one pixel
				nMaxFirstItem  / nMaxThumb;
		}
	}

	void UIScrollBar::MouseDownHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		last_mouse_ = arg.location;
		if (arg.buttons & MB_Left)
		{
			if (up_button_rc_.PtInRect(arg.location))
			{
				if (position_ > start_)
				{
					-- position_;
				}
				this->UpdateThumbRect();
				arrow_ = CLICKED_UP;
				arrow_ts_ = timer_.current_time();
				return;
			}

			// Check for click on down button

			if (down_button_rc_.PtInRect(arg.location))
			{
				if (position_ + page_size_ < end_)
				{
					++ position_;
				}
				this->UpdateThumbRect();
				arrow_ = CLICKED_DOWN;
				arrow_ts_ = timer_.current_time();
				return;
			}

			// Check for click on thumb

			if (thumb_rc_.PtInRect(arg.location))
			{
				drag_ = true;
				thumb_offset_y_ = arg.location.y() - thumb_rc_.top();
				return;
			}

			// Check for click on track

			if ((thumb_rc_.left() <= arg.location.x()) && (thumb_rc_.right() > arg.location.x()))
			{
				if ((thumb_rc_.top() > arg.location.y()) && (track_rc_.top() <= arg.location.y()))
				{
					this->Scroll(-static_cast<int>(page_size_ - 1));
				}
				else
				{
					if ((thumb_rc_.bottom() <= arg.location.y()) && (track_rc_.bottom() > arg.location.y()))
					{
						this->Scroll(static_cast<int>(page_size_ - 1));
					}
				}
			}
		}
	}

	void UIScrollBar::MouseUpHandler(UIDialog const & /*sender*/, MouseEventArg const & arg)
	{
		last_mouse_ = arg.location;
		if (arg.buttons & MB_Left)
		{
			drag_ = false;
			this->UpdateThumbRect();
			arrow_ = CLEAR;
		}
	}

	void UIScrollBar::Render()
	{
		// Check if the arrow button has been held for a while.
		// If so, update the thumb position to simulate repeated
		// scroll.
		if (arrow_ != CLEAR)
		{
			double dCurrTime = timer_.current_time();
			if (up_button_rc_.PtInRect(last_mouse_))
			{
				switch (arrow_)
				{
				case CLICKED_UP:
					if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - arrow_ts_)
					{
						this->Scroll(-1);
						arrow_ = HELD_UP;
						arrow_ts_ = dCurrTime;
					}
					break;

				case HELD_UP:
					if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - arrow_ts_)
					{
						this->Scroll(-1);
						arrow_ts_ = dCurrTime;
					}
					break;
				}
			}
			else
			{
				if (down_button_rc_.PtInRect(last_mouse_))
				{
					switch (arrow_)
					{
					case CLICKED_DOWN:
						if (SCROLLBAR_ARROWCLICK_DELAY < dCurrTime - arrow_ts_)
						{
							this->Scroll(1);
							arrow_ = HELD_DOWN;
							arrow_ts_ = dCurrTime;
						}
						break;

					case HELD_DOWN:
						if (SCROLLBAR_ARROWCLICK_REPEAT < dCurrTime - arrow_ts_)
						{
							this->Scroll(1);
							arrow_ts_ = dCurrTime;
						}
						break;
					}
				}
			}
		}

		UI_Control_State iState = UICS_Normal;

		if (!visible_)
		{
			iState = UICS_Hidden;
		}
		else
		{
			if (!enabled_ || !show_thumb_)
			{
				iState = UICS_Disabled;
			}
			else
			{
				if (is_mouse_over_)
				{
					iState = UICS_MouseOver;
				}
				else
				{
					if (has_focus_)
					{
						iState = UICS_Focus;
					}
				}
			}
		}


		// Background track layer
		UIElementPtr pElement = elements_[0];

		// Blend current color
		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, track_rc_);

		// Up Arrow
		pElement = elements_[1];

		// Blend current color
		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, up_button_rc_);

		// Down Arrow
		pElement = elements_[2];

		// Blend current color
		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, down_button_rc_);

		// Thumb button
		pElement = elements_[3];

		// Blend current color
		pElement->TextureColor().SetState(iState);
		this->GetDialog()->DrawSprite(*pElement, thumb_rc_);
	}

	void UIScrollBar::SetTrackRange(size_t nStart, size_t nEnd)
	{
		start_ = nStart;
		end_ = nEnd;
		this->Cap();
		this->UpdateThumbRect();
	}

	void UIScrollBar::Cap()  // Clips position at boundaries. Ensures it stays within legal range.
	{
		if ((position_ < start_) || (end_ - start_ <= page_size_))
		{
			position_ = start_;
		}
		else
		{
			if (position_ + page_size_ > end_)
			{
				position_ = end_ - page_size_;
			}
		}
	}
}
