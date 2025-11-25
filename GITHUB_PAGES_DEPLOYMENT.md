# GitHub Pages Deployment Guide
# QuakeCloneWASM - Publishing Instructions

## Prerequisites

1. **GitHub Account**: You need a GitHub account
2. **Git Repository**: Your project should be a Git repository
3. **GitHub Repository**: Create a repository on GitHub (if not already created)

## Step-by-Step Deployment Instructions

### 1. Initialize Git Repository (if not already done)

```bash
cd QuakeCloneWASM
git init
git add .
git commit -m "Initial commit - QuakeCloneWASM"
```

### 2. Create GitHub Repository

1. Go to [GitHub.com](https://github.com)
2. Click the **"+"** icon in the top right corner
3. Select **"New repository"**
4. Enter repository name (e.g., `webAssemblyQuakeClone` or `QuakeCloneWASM`)
5. Choose **Public** (required for free GitHub Pages)
6. **DO NOT** initialize with README, .gitignore, or license (you already have these)
7. Click **"Create repository"**

### 3. Connect Local Repository to GitHub

**Important**: The workflow assumes your repository root is the `QuakeCloneWASM` folder. If your repository root is `webAssemblyQuakeClone`, you'll need to update the workflow path.

```bash
# Navigate to QuakeCloneWASM directory (repository root)
cd QuakeCloneWASM

# Replace YOUR_USERNAME and YOUR_REPO_NAME with your actual values
git remote add origin https://github.com/YOUR_USERNAME/YOUR_REPO_NAME.git
git branch -M main
git push -u origin main
```

**Note**: If you create the repository at the `webAssemblyQuakeClone` level instead:
1. Update `.github/workflows/deploy-pages.yml` line 42
2. Change `path: './site'` to `path: './QuakeCloneWASM/site'`

### 4. Enable GitHub Pages

1. Go to your repository on GitHub
2. Click **"Settings"** tab (at the top of the repository)
3. Scroll down to **"Pages"** in the left sidebar
4. Under **"Source"**, select:
   - **"GitHub Actions"** (recommended - uses the workflow we created)
   - OR **"Deploy from a branch"** → select **"main"** → select **"/ (root)"** → then manually copy site files to root or use docs folder
5. Click **"Save"**

### 5. Trigger Deployment

The GitHub Actions workflow will automatically deploy when you:
- Push to the `main` or `master` branch
- OR manually trigger it:
  1. Go to **"Actions"** tab in your repository
  2. Select **"Deploy to GitHub Pages"** workflow
  3. Click **"Run workflow"** button
  4. Select branch (usually `main`)
  5. Click **"Run workflow"**

### 6. Access Your Published Demo

After deployment completes (usually 1-2 minutes):
- Your demo will be available at: `https://YOUR_USERNAME.github.io/YOUR_REPO_NAME/`
- You can find the exact URL in:
  - Repository **"Settings"** → **"Pages"** section
  - The workflow run output in **"Actions"** tab

## Important Notes

### File Paths
- The application uses relative paths (`./wasm/game.js`), which work correctly on GitHub Pages
- If your repository name is not the root (e.g., `username.github.io/repo-name`), paths should still work as they're relative

### Build Files
- The `site` directory contains all files needed for deployment
- Ensure `site/wasm/game.js` and `site/wasm/game.wasm` are committed to git
- The `.nojekyll` file ensures GitHub Pages serves all files correctly (including `_framework` folder)

### Updating the Demo
- After making changes, rebuild if needed: `build.bat`
- Commit and push changes: `git add . && git commit -m "Update" && git push`
- GitHub Actions will automatically redeploy

### Troubleshooting

**Issue: 404 errors or files not loading**
- Check that all files in `site` directory are committed
- Verify `.nojekyll` file exists in `site` directory
- Check GitHub Actions workflow logs for errors

**Issue: WASM files not loading**
- Ensure `site/wasm/game.wasm` and `site/wasm/game.js` are committed
- Check browser console for CORS or loading errors
- Verify file paths in `index.html` and `main.js` are relative

**Issue: Workflow not running**
- Check that GitHub Actions is enabled in repository settings
- Verify the workflow file is in `.github/workflows/` directory
- Check that you're pushing to `main` or `master` branch

## Alternative: Manual Deployment (if GitHub Actions doesn't work)

1. Create a `docs` folder in the repository root
2. Copy all contents from `site` folder to `docs` folder
3. In GitHub Pages settings, select **"Deploy from a branch"** → **"main"** → **"/docs"**
4. Commit and push the changes

## Repository Structure for GitHub Pages

```
QuakeCloneWASM/
├── .github/
│   └── workflows/
│       └── deploy-pages.yml    # GitHub Actions deployment workflow
├── site/
│   ├── .nojekyll               # Prevents Jekyll processing
│   ├── index.html              # Main entry point
│   ├── main.js                 # JavaScript bridge
│   ├── wasm/
│   │   ├── game.js             # Emscripten module
│   │   └── game.wasm           # WebAssembly binary
│   └── _framework/             # Blazor WebAssembly framework
└── ... (other project files)
```

