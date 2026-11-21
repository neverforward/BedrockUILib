# Style

Colours come from two layers:

- the `Theme` is the document: the palette the whole tree starts from (`setTheme`);
- a `Style` is one widget's own override: only the tokens it sets are replaced,
  everything else comes from the theme it inherited.

```cpp
auto group = std::make_unique<bul::Stack>(bul::Stack::Direction::Vertical);
group->style({.text = bul::Rgba::fromRgb(0x8CC4FF), .accent = bul::Rgba::fromRgb(0xFFB454)});
group->add(std::make_unique<bul::Label>("Inherited text colour")); // painted in the style colour
```

A container resolves its own style and passes the result to its children, so a
style on a `Stack` colours the whole subtree; a child that sets the same token
wins. A spec tree carries the same thing as `Common::style`.

| Token | Used by |
| --- | --- |
| `text` / `textMuted` | text and captions |
| `accent` / `invalid` | focus and error states |
| `primary` / `primaryHover` / `primaryText` | the default button |
| `secondary` / `secondaryHover` | the secondary button |
| `destructive` / `destructiveHover` / `destructiveText` | the destructive button |
| `button` / `buttonHover` | neutral surfaces |
| `input` / `inputHover` / `inputBorder` | text fields and select triggers |
| `panel` / `panelBorder` | panels and popups |
