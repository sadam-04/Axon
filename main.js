const { app, BrowserWindow, ipcMain, dialog } = require('electron');

const path = require('node:path');
const fs = require('node:fs').promises;

async function handleFileOpen() {
  const { canceled, filePaths } = await dialog.showOpenDialog({});
  if (!canceled && filePaths.length > 0) {
    const data = await fs.readFile(filePaths[0], 'utf-8');
    return data; // returned to renderer
  }
  return null;
}

const createWindow = () => {
  const win = new BrowserWindow({
    width: 800,
    height: 600,
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
    }
  });

  // Menu.setApplicationMenu(null);

  win.loadFile('index.html');
}

app.whenReady().then(() => {
  ipcMain.handle('dialog:openFile', handleFileOpen);
  
  createWindow();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});