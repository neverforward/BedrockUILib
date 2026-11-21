# Documentation site

These pages are a [VitePress](https://vitepress.dev) site. Every `.md` file in
this folder is a page; `.vitepress/config.mts` defines the navigation, the sidebar
and the two locales (English at the root, Chinese under `zh/`).

```bash
cd docs
npm install        # once
npm run dev        # live preview on http://localhost:5173
npm run build      # static site in docs/.vitepress/dist/
npm run preview    # serve the built site
```

## Layout

| Path | Page |
| --- | --- |
| `index.md` | Landing page |
| `guide/getting-started.md` | Build, test, deploy, use from another mod |
| `components.md` | The widget kit: layout, fields, every control, style layers, spec trees |
| `architecture.md` | Module map and the constraints that shaped it |
| `investigation-native-ui.md` | How native UI can be built on this client, with evidence |
| `zh/` | The same pages in Chinese |

The in-game gallery (**F6**) is the other half of the documentation: it is built
from the public API only, so it can never drift from the code.
