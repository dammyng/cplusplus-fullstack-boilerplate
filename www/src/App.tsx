import { useEffect } from 'react';
import './App.css'
import { useStore } from './stores/useStore';
import { observer } from 'mobx-react-lite';
import CardLayout from './components/CardLayout';
import Header from './components/Header';

function App() {
  const { themeStore } = useStore();

  useEffect(() => {
    const root = window.document.documentElement;
    if (themeStore.theme === 'dark') {
      root.classList.add('dark');
    } else {
      root.classList.remove('dark');
    }
  }, [themeStore.theme]);

  return (
    <div className="bg-white dark:bg-gray-900">
      <Header />
      <main className="p-4 flex items-center justify-center">
        <CardLayout />
      </main>
    </div>
  )
}

export default observer(App)
