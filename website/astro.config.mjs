import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import remarkMath from 'remark-math';
import rehypeKatex from 'rehype-katex';

export default defineConfig({
  markdown: {
    remarkPlugins: [remarkMath],
    rehypePlugins: [rehypeKatex],
  },
  integrations: [
    starlight({
      title: 'qseries',
      description: 'Exact q-series computation for partition theory, modular forms, and combinatorics',
      social: [
        { icon: 'github', label: 'GitHub', href: 'https://github.com/GarvanResearchGroup/qseries' },
      ],
      customCss: ['./src/styles/custom.css'],
      sidebar: [
        {
          label: 'Getting Started',
          items: [
            { slug: 'getting-started/installation' },
            { slug: 'getting-started/quick-start' },
          ],
        },
        {
          label: 'Reference Manual',
          items: [
            { slug: 'manual/bigint-frac' },
            { slug: 'manual/series-ops' },
            { slug: 'manual/q-functions' },
            { slug: 'manual/product-conversion' },
            { slug: 'manual/relations' },
            { slug: 'manual/sifting' },
            { slug: 'manual/repl-commands' },
          ],
        },
        {
          label: 'Tutorial',
          items: [
            { slug: 'tutorial' },
            { slug: 'tutorial/rogers-ramanujan' },
            { slug: 'tutorial/theta-functions' },
            { slug: 'tutorial/partition-identities' },
            { slug: 'tutorial/modular-equations' },
          ],
        },
        {
          label: 'Playground',
          items: [
            { slug: 'playground' },
          ],
        },
      ],
    }),
  ],
});
