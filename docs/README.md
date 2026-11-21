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
| `components/` | The widget kit, one page per topic plus the overview |
| `architecture.md` | Module map and the constraints that shaped it |
| `investigation-native-ui.md` | How native UI can be built on this client, with evidence |
| `zh/` | The same pages in Chinese, same paths under `zh/` |

The in-game gallery (**F6**) is the other half of the documentation: it is built
from the public API only, so it can never drift from the code.

## Deployment

`.github/workflows/docs.yml` builds the site and publishes it to GitHub Pages:

- a pull request touching `docs/` builds it, so a dead link fails the check;
- a push to `main` builds and deploys to <https://neverforward.github.io/BedrockUILib/>;
- `workflow_dispatch` deploys on demand.

One repository setting is required: **Settings → Pages → Source: GitHub Actions**.
The workflow passes `DOCS_BASE=/BedrockUILib/` because project pages are served
under the repository name; a local build keeps `/`.
