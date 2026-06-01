import { Activity, BarChart3, Search } from 'lucide-react';
import { NavLink, Outlet } from 'react-router-dom';

import { HealthBadge } from './HealthBadge';

const navItems = [
  { to: '/', label: 'Search', icon: Search },
  { to: '/analytics', label: 'Analytics', icon: BarChart3 },
];

export function Layout() {
  return (
    <div className="min-h-screen bg-slate-50 text-slate-950">
      <header className="border-b border-slate-200 bg-white">
        <div className="mx-auto flex max-w-7xl flex-col gap-4 px-4 py-4 sm:px-6 lg:flex-row lg:items-center lg:justify-between lg:px-8">
          <div className="flex items-center gap-3">
            <div className="grid h-10 w-10 place-items-center rounded-lg bg-teal-600 text-white">
              <Activity className="h-5 w-5" aria-hidden="true" />
            </div>
            <div>
              <p className="text-lg font-semibold tracking-normal">IntelliSearch</p>
              <p className="text-sm text-slate-500">Retrieval workbench</p>
            </div>
          </div>

          <div className="flex flex-wrap items-center gap-3">
            <nav className="flex rounded-lg border border-slate-200 bg-slate-100 p-1">
              {navItems.map((item) => {
                const Icon = item.icon;
                return (
                  <NavLink
                    key={item.to}
                    to={item.to}
                    className={({ isActive }) =>
                      [
                        'flex min-h-9 items-center gap-2 rounded-md px-3 text-sm font-medium transition',
                        isActive
                          ? 'bg-white text-slate-950 shadow-sm'
                          : 'text-slate-600 hover:text-slate-950',
                      ].join(' ')
                    }
                  >
                    <Icon className="h-4 w-4" aria-hidden="true" />
                    {item.label}
                  </NavLink>
                );
              })}
            </nav>
            <HealthBadge />
          </div>
        </div>
      </header>

      <main className="mx-auto max-w-7xl px-4 py-6 sm:px-6 lg:px-8">
        <Outlet />
      </main>
    </div>
  );
}
