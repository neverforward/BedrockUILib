#include "gtest/gtest.h"

#include <memory>

#include "FakeCanvas.h"

#include "bul/Theme.h"
#include "bul/Widget.h"

namespace {

[[nodiscard]] bul::Theme testTheme() {
    bul::Theme theme = bul::Theme::neutral();
    theme.fontScale  = 1.f;
    theme.gap        = 6.f;
    return theme;
}

/// A content block with a fixed measured size and no drawing of its own.
class Block final : public bul::Widget {
public:
    Block(float width, float height) : mSize{width, height} {}

    [[nodiscard]] bul::Size measureContent(float, bul::Theme const&, bul::Canvas const&) const override {
        return mSize;
    }
    void paintContent(bul::Canvas&, bul::Theme const&) const override {}


private:
    bul::Size mSize;
};

} // namespace

TEST(ScrollView, MeasureCapsTheContentAtTheViewport) {
    bul::test::FakeCanvas canvas;

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));

    bul::Size const size = view.measure(0.f, testTheme(), canvas);
    EXPECT_EQ(size.width, 80.f);
    EXPECT_EQ(size.height, 100.f);
}

TEST(ScrollView, WithoutAViewportTheContentFits) {
    bul::test::FakeCanvas canvas;

    bul::ScrollView view;
    view.content(std::make_unique<Block>(80.f, 300.f));

    EXPECT_EQ(view.measure(0.f, testTheme(), canvas).height, 300.f);
}

TEST(ScrollView, LayoutShiftsTheContentByTheOffset) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    auto         block   = std::make_unique<Block>(80.f, 300.f);
    bul::Widget* content = block.get();
    view.content(std::move(block));

    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);
    EXPECT_EQ(content->bounds().y, 0.f);

    view.scrollBy(40.f);
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);
    EXPECT_EQ(content->bounds().y, -40.f);
}

TEST(ScrollView, ScrollIsClampedToTheContent) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    view.scrollBy(1000.f);
    EXPECT_EQ(view.offset(), 200.f); // 300 content - 100 viewport
    view.scrollBy(-1000.f);
    EXPECT_EQ(view.offset(), 0.f);
}

TEST(ScrollView, PaintClipsToTheViewport) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);
    view.paint(canvas, theme);

    ASSERT_EQ(canvas.clipPushes.size(), 1u);
    EXPECT_EQ(canvas.clipPushes[0], (bul::Rect{0.f, 0.f, 200.f, 100.f}));
}

TEST(ScrollView, DrawsTheScrollbarOnlyWhenItScrolls) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView scrolling;
    scrolling.viewport(100.f);
    scrolling.content(std::make_unique<Block>(80.f, 300.f));
    scrolling.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);
    scrolling.paint(canvas, theme);
    EXPECT_EQ(canvas.fills.size(), 2u); // the track and the thumb

    bul::test::FakeCanvas fitsCanvas;
    bul::ScrollView       fits;
    fits.viewport(100.f);
    fits.content(std::make_unique<Block>(80.f, 60.f));
    fits.layout({0.f, 0.f, 200.f, 100.f}, theme, fitsCanvas);
    fits.paint(fitsCanvas, theme);
    EXPECT_TRUE(fitsCanvas.fills.empty());
}

TEST(ScrollView, TheThumbGrowsWithTheVisibleFraction) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);
    view.paint(canvas, theme);

    ASSERT_EQ(canvas.fills.size(), 2u);
    // The track is the full viewport; the thumb is a third of it.
    EXPECT_EQ(canvas.fills[0].rect.height, 100.f);
    EXPECT_FLOAT_EQ(canvas.fills[1].rect.height, 100.f / 3.f);
}

TEST(ScrollView, TheWheelScrollsOneStep) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.wheelStep(20.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    EXPECT_TRUE(view.wheel({10.f, 10.f}, 1.f));
    EXPECT_EQ(view.offset(), 20.f);
    EXPECT_TRUE(view.wheel({10.f, 10.f}, -1.f));
    EXPECT_EQ(view.offset(), 0.f);
}

TEST(ScrollView, TheWheelOutsideTheViewportIsIgnored) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    EXPECT_FALSE(view.wheel({500.f, 500.f}, 1.f));
}

TEST(ScrollView, AContainerForwardsTheWheelToTheScrollView) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::Stack column{bul::Stack::Direction::Vertical};
    auto       view = std::make_unique<bul::ScrollView>();
    view->viewport(100.f);
    view->wheelStep(20.f);
    view->content(std::make_unique<Block>(80.f, 300.f));
    bul::ScrollView* raw = view.get();
    column.add(std::move(view));
    column.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    EXPECT_TRUE(column.wheel({10.f, 10.f}, 1.f));
    EXPECT_EQ(raw->offset(), 20.f);
}

TEST(ScrollView, TheBarLaneClaimsThePointer) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    EXPECT_EQ(view.hitTest({198.f, 50.f}), &view);
}

TEST(ScrollView, DraggingTheThumbScrolls) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    // The thumb is at the top: grab it and pull it down.
    ASSERT_TRUE(view.press({198.f, 10.f}));
    view.drag({198.f, 60.f});
    EXPECT_GT(view.offset(), 0.f);

    view.drag({198.f, 500.f}); // past the end
    EXPECT_EQ(view.offset(), 200.f);

    view.release();
    view.drag({198.f, 0.f});
    EXPECT_EQ(view.offset(), 200.f); // a drag after the release does nothing
}

TEST(ScrollView, PressingOutsideTheBarIsNotCaptured) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    view.content(std::make_unique<Block>(80.f, 300.f));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    EXPECT_FALSE(view.press({50.f, 50.f}));
}

TEST(ScrollView, TheContentKeepsAGutterBeforeTheBar) {
    bul::test::FakeCanvas canvas;
    bul::Theme const      theme = testTheme();

    bul::ScrollView view;
    view.viewport(100.f);
    auto         block   = std::make_unique<Block>(80.f, 300.f);
    bul::Widget* content = block.get();
    view.content(std::move(block));
    view.layout({0.f, 0.f, 200.f, 100.f}, theme, canvas);

    // The bar lane is 4 units plus 4 units of gap.
    EXPECT_EQ(content->bounds().width, 192.f);

    bul::test::FakeCanvas fitsCanvas;
    bul::ScrollView       fits;
    fits.viewport(100.f);
    auto         small  = std::make_unique<Block>(80.f, 60.f);
    bul::Widget* fitted = small.get();
    fits.content(std::move(small));
    fits.layout({0.f, 0.f, 200.f, 100.f}, theme, fitsCanvas);
    EXPECT_EQ(fitted->bounds().width, 200.f); // nothing to scroll, no lane
}