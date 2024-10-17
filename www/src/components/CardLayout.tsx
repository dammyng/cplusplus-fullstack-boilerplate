// src/components/CardLayout.tsx
import React from 'react';

const CardLayout: React.FC = () => {
  return (

    <div className="flex flex-wrap -mx-4 container">
      <div className="w-full px-4 mb-4 md:w-1/2 lg:w-1/4 lg:mb-0">
        <div className="flex flex-col bg-gray-200 dark:bg-gray-700 rounded-lg shadow-md p-4 h-full">
          <p className="text-center">Left Column</p>

        </div>
      </div>

      <div className="w-full px-4 mb-4 md:w-1/2 lg:w-1/2 lg:mb-0">
        <div className="flex flex-col bg-gray-300 dark:bg-gray-600 rounded-lg shadow-md p-4 h-full">
          <p className="text-center">Middle Column</p>
        </div>
      </div>

      <div className="hidden lg:block w-full px-4 lg:w-1/4 lg:mb-0">
        <div className="flex flex-col bg-gray-200 dark:bg-gray-700 rounded-lg shadow-md p-4 h-full">
          <p className="text-center">Right Column</p>
        </div>
      </div>
    </div>
  );
};

export default CardLayout;