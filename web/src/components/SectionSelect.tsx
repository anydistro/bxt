/* === This file is part of bxt ===
 *
 *   SPDX-FileCopyrightText: 2023 Artem Grinev <agrinev@manjaro.org>
 *   SPDX-License-Identifier: AGPL-3.0-or-later
 *
 */
import Select, {
    GroupBase,
    SelectComponentsConfig,
    StylesConfig,
    components
} from "react-select";
import { HTMLAttributes, useCallback } from "react";
import {
    faCodeBranch,
    faCubes,
    faMicrochip
} from "@fortawesome/free-solid-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import * as daisyui from "react-daisyui";
import { IconProp } from "@fortawesome/fontawesome-svg-core";
import { ClassNamesArg } from "@emotion/react";
import { SectionUtils } from "../utils/SectionUtils";

type SectionSelectorProps = HTMLAttributes<HTMLDivElement> & {
    sections: Section[];
    selectedSection?: Section;
    onSelected?: (section: Section) => void;
    disabled?: boolean | boolean[];
};
export default function SectionSelect({
    sections,
    selectedSection,
    onSelected,
    disabled
}: SectionSelectorProps) {
    const updateSection = useCallback(
        (update: any) => {
            if (onSelected) onSelected({ ...selectedSection, ...update });
        },
        [onSelected, selectedSection]
    );

    interface IOption {
        value?: string;
        label?: string;
    }

    const makeValue = useCallback(
        (value: string | undefined): IOption => ({
            value: value,
            label: value
        }),
        []
    );

    const makeComponents = useCallback(
        <TGroup extends GroupBase<IOption>>(
            icon: IconProp,
            classNames: ClassNamesArg = ""
        ): SelectComponentsConfig<IOption, false, TGroup> => ({
            Control: ({ children, ...rest }) => (
                <daisyui.Dropdown>
                    <components.Control
                        className={
                            "input input-bordered input-sm bg-base-100 " +
                            classNames
                        }
                        {...rest}
                    >
                        <FontAwesomeIcon className="px-2" icon={icon} />
                        {children}
                    </components.Control>
                </daisyui.Dropdown>
            ),
            Menu: ({ children, innerProps, ...rest }) => (
                <daisyui.Dropdown.Menu
                    className="z-10 fixed bg-base-100 menu-sm"
                    {...rest}
                >
                    <div {...innerProps}>{children}</div>
                </daisyui.Dropdown.Menu>
            ),
            Option: ({ children, innerProps, isSelected, ...rest }) => (
                <daisyui.Menu.Item>
                    <div className={isSelected ? "active" : ""} {...innerProps}>
                        {children}
                    </div>
                </daisyui.Menu.Item>
            )
        }),
        []
    );

    const makeStyles = useCallback(
        (disabled?: boolean): StylesConfig<IOption> => ({
            control: (base) => ({
                ...base,
                minHeight: 32,
                pointerEvents: disabled ? "none" : undefined,
                opacity: disabled ? "0.4" : undefined
            }),
            dropdownIndicator: (base) => ({
                ...base,
                paddingTop: 0,
                paddingBottom: 0
            }),
            clearIndicator: (base) => ({
                ...base,
                paddingTop: 0,
                paddingBottom: 0
            })
        }),
        []
    );

    const isDisabled = useCallback(
        (n: number) => {
            if (disabled == undefined) {
                return false;
            }
            if (typeof disabled == "boolean") {
                return disabled as boolean;
            }
            return (
                (disabled as boolean[]).length > n && (disabled as boolean[])[n]
            );
        },
        [disabled]
    );

    return (
        <span className="flex justify-stretch items-center w-full space-x-1">
            <Select<IOption>
                unstyled={true}
                components={makeComponents(faCodeBranch)}
                styles={makeStyles(isDisabled(0))}
                value={makeValue(selectedSection?.branch)}
                options={SectionUtils.branches(sections).map(makeValue)}
                onChange={(value) => updateSection({ branch: value?.value })}
            />
            <Select<IOption>
                unstyled={true}
                components={makeComponents(faCubes)}
                styles={makeStyles(isDisabled(1))}
                value={makeValue(selectedSection?.repository)}
                options={SectionUtils.reposForBranch(
                    sections,
                    selectedSection?.branch
                ).map(makeValue)}
                onChange={(value) =>
                    updateSection({ repository: value?.value })
                }
            />
            <Select<IOption>
                unstyled={true}
                components={makeComponents(faMicrochip)}
                styles={makeStyles(isDisabled(2))}
                value={makeValue(selectedSection?.architecture)}
                options={SectionUtils.architecturesForBranchAndRepo(
                    sections,
                    selectedSection?.branch,
                    selectedSection?.repository
                ).map(makeValue)}
                onChange={(value) =>
                    updateSection({ architecture: value?.value })
                }
            />
        </span>
    );
}
