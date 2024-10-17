import React from 'react';
import { observer } from 'mobx-react-lite';
import { useStore } from '../stores/useStore';
import { Switch, Disclosure } from '@headlessui/react';
import { XMarkIcon, QueueListIcon } from '@heroicons/react/24/outline';

const Header: React.FC = observer(() => {
  const { themeStore } = useStore();

  return (
    <Disclosure as="nav" className="bg-white dark:bg-gray-800 shadow">
      {({ open }) => (
        <>
          <div className="container mx-auto px-4 py-4 flex justify-between items-center">
            <div className="text-2xl font-bold text-gray-900 dark:text-white">
              My App
            </div>

            <nav className="hidden md:flex space-x-4">
              <a href="#" className="text-gray-800 dark:text-gray-200 hover:underline">
                Home
              </a>
              <a href="#" className="text-gray-800 dark:text-gray-200 hover:underline">
                About
              </a>
              <a href="#" className="text-gray-800 dark:text-gray-200 hover:underline">
                Contact
              </a>
            </nav>

            <div className="flex items-center space-x-4">
              {/* Theme Switcher */}
              <Switch
                checked={themeStore.theme === 'dark'}
                onChange={themeStore.toggleTheme}
                className={`${
                  themeStore.theme === 'dark' ? 'bg-blue-600' : 'bg-gray-200'
                } inline-flex items-center h-6 rounded-full w-11 focus:outline-none transition`}
              >
                <span
                  className={`${
                    themeStore.theme === 'dark' ? 'translate-x-6' : 'translate-x-1'
                  } inline-block w-4 h-4 transform bg-white rounded-full transition-transform`}
                />
              </Switch>

              <Disclosure.Button
                className="text-gray-800 dark:text-gray-200 focus:outline-none md:hidden"
                aria-label="Toggle mobile menu"
              >
                {open ? (
                  <XMarkIcon className="h-6 w-6" aria-hidden="true" />
                ) : (
                  <QueueListIcon className="h-6 w-6" aria-hidden="true" />
                )}
              </Disclosure.Button>
            </div>
          </div>

          <Disclosure.Panel className="md:hidden">
            <nav className="px-4 pt-2 pb-3 space-y-1">
              <a href="#" className="block text-gray-800 dark:text-gray-200 hover:underline">
                Home
              </a>
              <a href="#" className="block text-gray-800 dark:text-gray-200 hover:underline">
                About
              </a>
              <a href="#" className="block text-gray-800 dark:text-gray-200 hover:underline">
                Contact
              </a>
            </nav>
          </Disclosure.Panel>
        </>
      )}
    </Disclosure>
  );
});

export default Header;