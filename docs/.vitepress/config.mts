import { defineConfig } from 'vitepress'

const repo = 'https://github.com/neverforward/BedrockUILib'

const editLink = { pattern: `${repo}/edit/main/docs/:path` }

// GitHub Pages serves project sites under /<repo>/, so the workflow passes that
// base in; a local build or the dev server keeps '/' .
const base = process.env.DOCS_BASE || '/'

export default defineConfig({
  title: 'BedrockUILib',
  base,
  description: 'Native widgets for Minecraft Bedrock client mods',
  cleanUrls: true,
  head: [['meta', { name: 'theme-color', content: '#59A9FF' }]],

  themeConfig: {
    socialLinks: [{ icon: 'github', link: repo }],
    editLink: { ...editLink, text: 'Edit this page' },
    search: { provider: 'local' },

    nav: [
      { text: 'Guide', link: '/guide/getting-started' },
      { text: 'Components', link: '/components/' },
      { text: 'Architecture', link: '/architecture' },
      { text: 'Investigation', link: '/investigation-native-ui' },
    ],

    sidebar: [
      {
        text: 'Guide',
        items: [
          { text: 'Getting started', link: '/guide/getting-started' },
          { text: 'Architecture', link: '/architecture' },
          { text: 'Native UI investigation', link: '/investigation-native-ui' },
        ],
      },
      {
        text: 'Components',
        items: [
          { text: 'Overview', link: '/components/' },
          { text: 'Layout', link: '/components/layout' },
          { text: 'Widget base', link: '/components/widget-base' },
          { text: 'Style', link: '/components/style' },
          { text: 'Stylesheets', link: '/components/stylesheets' },
          { text: 'Spec trees', link: '/components/spec-trees' },
          { text: 'Form field', link: '/components/form-field' },
          { text: 'Text', link: '/components/text' },
          { text: 'Input', link: '/components/input' },
          { text: 'Select', link: '/components/select' },
          { text: 'Checkbox', link: '/components/checkbox' },
          { text: 'Switch', link: '/components/switch' },
          { text: 'Radio group', link: '/components/radio-group' },
          { text: 'Theming', link: '/components/theming' },
          { text: 'API export', link: '/components/api-export' },
        ],
      },
    ],
  },

  locales: {
    root: { label: 'English', lang: 'en-US' },
    zh: {
      label: '简体中文',
      lang: 'zh-CN',
      themeConfig: {
        editLink: { ...editLink, text: '在 GitHub 编辑此页' },
        search: { provider: 'local' },
        nav: [
          { text: '指南', link: '/zh/guide/getting-started' },
          { text: '组件', link: '/zh/components/' },
          { text: '架构', link: '/zh/architecture' },
          { text: '调研', link: '/zh/investigation-native-ui' },
        ],
        sidebar: [
          {
            text: '指南',
            items: [
              { text: '快速开始', link: '/zh/guide/getting-started' },
              { text: '架构', link: '/zh/architecture' },
              { text: '原生 UI 调研', link: '/zh/investigation-native-ui' },
            ],
          },
          {
            text: '组件',
            items: [
              { text: '总览', link: '/zh/components/' },
              { text: '布局', link: '/zh/components/layout' },
              { text: '控件基类', link: '/zh/components/widget-base' },
              { text: '样式', link: '/zh/components/style' },
              { text: '样式表', link: '/zh/components/stylesheets' },
              { text: 'Spec 树', link: '/zh/components/spec-trees' },
              { text: '表单字段', link: '/zh/components/form-field' },
              { text: 'Label', link: '/zh/components/text' },
              { text: 'Input', link: '/zh/components/input' },
              { text: 'Select', link: '/zh/components/select' },
              { text: 'Checkbox', link: '/zh/components/checkbox' },
              { text: 'Switch', link: '/zh/components/switch' },
              { text: 'Radio Group', link: '/zh/components/radio-group' },
              { text: '主题', link: '/zh/components/theming' },
              { text: 'API 导出', link: '/zh/components/api-export' },
            ],
          },
        ],
      },
    },
  },
})
