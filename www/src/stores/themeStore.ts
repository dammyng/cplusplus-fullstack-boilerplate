// src/stores/themeStore.ts
import { makeAutoObservable } from 'mobx';

export class ThemeStore {
  theme: 'light' | 'dark' = 'light';

  constructor() {
    makeAutoObservable(this);
    const savedTheme = localStorage.getItem('theme') as 'light' | 'dark' | null;
    if (savedTheme) {
      this.theme = savedTheme;
    }
  }

  toggleTheme = (): void => {
    this.theme = this.theme === 'light' ? 'dark' : 'light';
    localStorage.setItem('theme', this.theme);
  };
}

export const themeStore = new ThemeStore();