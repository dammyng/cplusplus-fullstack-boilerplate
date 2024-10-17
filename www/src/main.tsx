import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import App from './App.tsx'
import './index.css'
import { rootStore, RootStoreContext } from './stores/rootStore.ts'

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <RootStoreContext.Provider value={rootStore}>
      <App />
    </RootStoreContext.Provider>
  </StrictMode>,
)
