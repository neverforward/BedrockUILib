# Stylesheets

`StyleSheet` is the kit's stylesheet: rules select components and declare properties.

```cpp
using namespace bul::literals;

bul::StyleSheet sheet;
sheet.className("danger").set({.backgroundColor = bul::Rgba::fromRgb(0xE5484D)});
sheet.type("Label").set({.fontSize = 14_px});
sheet.type("Stack").child().type("Label").set({.color = ...});
sheet.type("Input").pseudo("disabled").set({.color = ...});
bul::setStyleSheet(std::move(sheet));
```

## Selectors

| Written as | Selector | Matches |
| --- | --- | --- |
| `.type("Button")`, `.type<bul::Button>()` | `Button` | widget type |
| `.className("danger")` | `.danger` | class name |
| `.id("submit")` | `#submit` | id |
| `.any()` | `*` | everything |
| `.type("A").type("B")` | `A B` | a `B` with an `A` ancestor |
| `.type("A").child().type("B")` | `A > B` | a `B` whose parent is an `A` |
| `.type("A").adjacent().type("B")` | `A + B` | a `B` right after an `A` |
| `.type("A").sibling().type("B")` | `A ~ B` | a `B` after any `A` |
| `.attr("kind")` | `[kind]` | the attribute exists |
| `.attr("kind", "text")` | `[kind=text]` | attribute equals |
| `.attr("tags", "hot", Word)` | `[tags~=hot]` | one word of a space-separated list |
| `.attr("lang", "en", PrefixDash)` | `[lang\|=en]` | `en` or `en-...` |
| `.attr("src", "https", StartsWith)` | `[src^=https]` | value starts with |
| `.attr("src", ".pdf", EndsWith)` | `[src$=.pdf]` | value ends with |
| `.attr("src", "bul", Contains)` | `[src*=bul]` | value contains |

Attribute operators are `Attribute::Match`: `Exists`, `Equals`, `Word`,
`PrefixDash`, `StartsWith`, `EndsWith`, `Contains`.

## Pseudo classes

The pseudo-class match is `pseudo(name, argument)`:

```cpp
sheet.type("Input").pseudo("disabled").set({...});           // :disabled
sheet.type("Label").pseudo("nth-child", "2").set({...});     // :nth-child(2)
sheet.type("Label").pseudo("not", ".quiet").set({...});      // :not(.quiet)
sheet.type("Stack").pseudo("has", "Label").set({...});       // :has(Label)
```

| Group | Names |
| --- | --- |
| state | `hover`, `focus`, `active`, `disabled`, `enabled`, `checked`, `invalid`, `valid`, `required`, `optional`, `read-only`, `read-write` |
| position | `first-child`, `last-child`, `only-child`, `nth-child(n)`, `nth-last-child(n)`, `first-of-type`, `last-of-type`, `only-of-type`, `nth-of-type(n)`, `nth-last-of-type(n)` |
| structure | `root`, `empty` |
| logical | `not(selector)`, `is(selector)`, `where(selector)`, `has(selector)` |

`nth-child` takes `2`, `odd`, `even` or `an+b`.

The pseudo elements (`::before`, `::after`, `::first-letter`, `::first-line`) are
not modelled: the kit paints whole widgets, not text runs with generated
content. `:link` / `:visited` / `:target` / `:lang()` have no equivalent here
either.

## Multi-Selector

`rule(...)` takes a comma-separated list, and each part becomes its own rule:

```cpp
sheet.rule("Button, Label.note", {.color = ...});
```

## Lengths

The numeric properties (`fontSize`, `gap`, `borderWidth`) take a `Length`: a
number and a unit. A bare number is a GUI unit, so `4.f` and `4_px` are the
same. `padding` takes a `Padding`, one length per side:
`Padding::all(4_px)` or `Padding::symmetric(1_em, 4_px)`, and a plain `Insets`
converts into it.

| Unit | Suffix | Resolves to |
| --- | --- | --- |
| px | `14_px` | GUI units, 1:1 |
| em | `1.5_em` | 1.5 times the theme's base font height |
| ex | `2_ex` | half a font height each |
| % | `50_pct` | hundredths of the base font height |
| in | `0.5_in` | 96 GUI units per inch |
| cm / mm | `2.54_cm`, `25.4_mm` | 96 units per inch, converted |
| pt / pc | `12_pt`, `1_pc` | 96/72 per point, 16 per pica |

The suffix carries an underscore because clang refuses literal operators whose
suffix does not start with one (`14px` cannot work), and `%` is an operator, so
the percent unit is spelled `_pct`. `using namespace bul::literals;` brings them
all in.

Absolute units are fixed; `em`, `ex` and `%` resolve against the theme's base
font height (`Theme::fontSize`, 9 GUI units by default) scaled by `fontScale`,
because there is no containing block to measure against while a style is
resolved.

## Resolution

Control resolution order: Inherited theme > Matched rules > Inline `style()`.