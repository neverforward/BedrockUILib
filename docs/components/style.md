# Style

Visuals come from two layers:

- the `Theme` is the document, the palette the whole tree starts from (`setTheme`);
- a `Style` is one widget's own override, the kit's `style=""` attribute. Only the
  properties it sets are replaced, everything else comes from the theme it
  inherited.

```cpp
auto group = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
group->style({.color = bul::Rgba::fromRgb(0x8CC4FF), .gap = 12.f});
group->add(std::make_unique<bul::Label>("Inherited text colour")); // painted in the style colour
```

## Properties

Every property follows the stylesheet vocabulary: it means what its namesake means, and it replaces the
component's own default for that slot.

| Property | Web name | Meaning |
| --- | --- | --- |
| `color` | `color` | text colour |
| `backgroundColor` | `background-color` | the widget's own surface; put it on a `:hover` rule for the hover colour |
| `borderColor` | `border-color` | border colour, on panels and fields alike |
| `borderWidth` | `border-width` | border stroke width |
| `padding` | `padding` | inner spacing (`Padding`, one length per side) |
| `margin` | `margin` | outer spacing, the space the layout reserves around the widget |
| `gap` | `gap` | space between a container's children |
| `width` / `height` | `width` / `height` | fixed size |
| `minWidth` / `minHeight` / `maxWidth` / `maxHeight` | `min-*` / `max-*` | size bounds |
| `fontSize` | `font-size` | text height, a `Length` |
| `lineHeight` | `line-height` | line advance, a `Length` |
| `textAlign` | `text-align` | `Align::Left` / `Center` / `Right` |
| `verticalAlign` | `vertical-align` | `TextVertical::Top` / `Middle` / `Bottom`, for a text block |
| `textTransform` | `text-transform` | `TextTransform::Uppercase` / `Lowercase` / `Capitalize` |
| `whiteSpace` | `white-space` | `WhiteSpace::Normal` wraps, `NoWrap` / `Pre` do not |
| `visibility` | `visibility` | `Hidden` paints nothing but keeps the space |
| `accentColor` | `accent-color` | the fill of an on control: checkbox, switch, focus ring |
| `borderRadius` | `border-radius` | rounded surface and border corners |
| `opacity` | `opacity` | alpha for the widget and everything it paints |
| `display` | `display` | `Display::None` drops the widget from the layout |
| `letterSpacing` | `letter-spacing` | extra advance between glyphs |
| `fontWeight` | `font-weight` | `Bold` draws the run as `§l...§r` |
| `fontStyle` | `font-style` | `Italic` draws the run as `§o...§r` |
| `obfuscated` | (kit extension) | `true` draws the run as `§k...§r`, the random glyphs |

A container with `backgroundColor` / `borderColor` paints a box behind its
children, the way a `div` does; `Stack` and `Grid` are the containers that do this
today.

Hover is not a property: it is the `:hover` pseudo class in a rule, which is how
a stylesheet writes it.

```cpp
bul::setStyleSheet(bul::StyleSheet{
    {"Button", {.backgroundColor = ..., .color = ...}},
    {"Button:hover", {.backgroundColor = ...}},
});
```

## The palette

Colors assigned by role are located in `Theme`, not in `Style`. Change them using `setTheme`, or set the style properties mapped to them in the stylesheet; see [Stylesheets](/components/stylesheets).